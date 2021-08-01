//
// hookdll.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include <memory>
#include <sstream>
#include <map>
#include <thread>
#include <optional>

#define NO_SOL
#include "tools.h"
#include "apihook.h"
#include "hookdll.h"

struct FatalException{
    DWORD error;

    FatalException():error(::GetLastError()){};
    FatalException(const FatalException&) = default;
    FatalException(FatalException&&) = default;
    ~FatalException() = default;
};

//============================================================================================
// data shared between all processses
//============================================================================================
struct CapturedWindowContext{
    enum class Status{
        FREE = 0,
        CAPTURED,
        CLOSED,
    };

    HWND hWnd;
    Status status;
    int x;
    int y;
    int cx;
    int cy;
    int alpha;
    HWND hWndInsertAfter;
};

struct UpdateCounter{
    int64_t from_leader;
    int64_t from_follower;
};

#pragma comment( linker, "/SECTION:.shared,RWS" )
#pragma data_seg( ".shared" )
static HHOOK hookHandle = 0;
static UpdateCounter update_counter = {0, 0};
static CapturedWindowContext captured_windows_ctx[MAX_CAPTURED_WINDOW] = {0};
#pragma data_seg()

LRESULT CALLBACK hookProc(int nCode, WPARAM wParam, LPARAM lParam);

//============================================================================================
// utilities
//============================================================================================
class LockHolder{
protected:
    HANDLE mutex;
    bool is_locked = false;
public:
    LockHolder() = delete;
    LockHolder(HANDLE mutex, bool lock=true) : mutex(mutex), is_locked(lock){
        if (lock){
            WaitForSingleObject(mutex, INFINITE);
        }
    };
    LockHolder(const LockHolder&) = delete;
    LockHolder(LockHolder&& src) : mutex(src.mutex), is_locked(src.is_locked){
        src.mutex = nullptr;
        src.is_locked = false;
    };
    ~LockHolder(){
        unlock();
    };

    void lock(){
        if (!is_locked){
            WaitForSingleObject(mutex, INFINITE);
            is_locked = true;
        }
    }

    void unlock(){
        if (is_locked){
            ReleaseMutex(mutex);
            is_locked = false;
        }
    }
};

//============================================================================================
// Base class for managing captured windows
//============================================================================================
enum class ControllMessageDword : DWORD{
    start_capture,
    end_capture,
    change_attribute,
};

static void assert(bool condition){
    if (!condition){
        throw FatalException();
    }
}

class Manager {
protected:
    bool isOrigin;
    WinHandle mutex;
    WinHandle event;
    UINT controlMessage;

public:
    Manager() = delete;
    Manager(const Manager&) = delete;
    Manager(Manager&&) = delete;

    Manager(bool isOrigin, HMODULE hModule) : isOrigin(isOrigin){
        static wchar_t nameBuf[MAX_PATH + 1];
        auto namelen = ::GetModuleFileNameW(hModule, nameBuf, sizeof(nameBuf) / sizeof(*nameBuf));
        assert(namelen != 0);
        for (auto c = nameBuf; *c; c++){
            if (*c == L'\\'){
                *c = L'_';
            }
        }
        std::wstring moduleName(nameBuf);
        auto mutexName = std::move(moduleName + L"mutex");
        auto eventName = std::move(moduleName + L"event");
        if (isOrigin){
            mutex = ::CreateMutexW(nullptr, false, mutexName.c_str());
            assert(mutex);
            event = ::CreateEventW(nullptr, true, false, eventName.c_str());
            assert(event);
        }else{
            mutex = ::OpenMutexW( MUTEX_ALL_ACCESS, false, mutexName.c_str());
            assert(mutex);
            event = ::OpenEventW(EVENT_ALL_ACCESS, false, eventName.c_str());
            assert(event);
        }
        controlMessage = ::RegisterWindowMessageW(moduleName.c_str());
        assert(controlMessage);
    }

    ~Manager() = default;

    UINT getControlMessageCode() const {return controlMessage;};
};

//============================================================================================
// Managing captured windows in fsmapper process
//    This class behaves as singleton pateern
//============================================================================================
class LeadManager : public Manager{
protected:
    WINDOW_CLOSE_CALLBACK callback;
    void* callback_ctx;
    std::map<HWND, int> captured_windows;
    bool should_stop = false;
    std::thread event_receiver;

public:
    LeadManager() = delete;
    LeadManager(HMODULE hModule, WINDOW_CLOSE_CALLBACK callback, void* context) : 
        Manager(true, hModule), callback(callback), callback_ctx(context){
        auto current_counter = update_counter;
        ::hookHandle = ::SetWindowsHookExW(WH_CALLWNDPROC, hookProc, hModule, 0);
        assert(hookHandle);
        event_receiver = std::move(std::thread([this, current_counter](){
            auto last_update_counter = current_counter;
            LockHolder lock(mutex);
            while (true){
                lock.unlock();
                ::WaitForSingleObject(event, INFINITE);
                lock.lock();
                ::ResetEvent(event);
                if (should_stop){
                    break;
                }else if (::update_counter.from_follower != last_update_counter.from_follower){
                    last_update_counter = ::update_counter;
                    auto itr = captured_windows.begin();
                    while ( itr != captured_windows.end()){
                        auto& target = captured_windows_ctx[itr->second];
                        if (target.status == CapturedWindowContext::Status::CLOSED){
                            auto hWnd = target.hWnd;
                            target.status = CapturedWindowContext::Status::FREE;
                            captured_windows.erase(itr);
                            ::update_counter.from_leader++;
                            last_update_counter = ::update_counter;
                            lock.unlock();
                            this->callback(hWnd, this->callback_ctx);
                            lock.lock();
                            if (last_update_counter.from_leader != ::update_counter.from_leader ||
                                last_update_counter.from_follower != ::update_counter.from_follower){
                                itr = captured_windows.begin();
                            }else{
                                itr++;
                            }
                        }else{
                            itr++;
                        }
                    }
                }
            }
        }));
    };

    ~LeadManager(){
        LockHolder lock(mutex);
        should_stop = true;
        ::SetEvent(event);
        lock.unlock();
        event_receiver.join();
        for (auto item : captured_windows){
            captured_windows_ctx[item.second].status = CapturedWindowContext::Status::FREE;
            ::SendMessageW(item.first, controlMessage, static_cast<DWORD>(ControllMessageDword::end_capture), 0);
        }
        ::UnhookWindowsHookEx(hookHandle);
        ::hookHandle = 0;
    };

    bool addCapture(HWND hWnd){
        LockHolder lock(mutex);
        if (captured_windows.count(hWnd) > 0){
            // already captured
            return false;
        }
        auto i = 0;
        for (; i < MAX_CAPTURED_WINDOW; i++){
            if (captured_windows_ctx[i].status == CapturedWindowContext::Status::FREE){
                break;
            }
        }
        if (i >= MAX_CAPTURED_WINDOW){
            // no more able to capture
            return false;
        }
        captured_windows.emplace(hWnd, i);
        captured_windows_ctx[i].hWnd = hWnd;
        captured_windows_ctx[i].status = CapturedWindowContext::Status::CAPTURED;
        lock.unlock();
        ::SendMessageW(hWnd, controlMessage, static_cast<DWORD>(ControllMessageDword::start_capture), 0);
        return true;
    };

    bool releaseCapture(HWND hWnd){
        LockHolder lock(mutex);
        if (captured_windows.count(hWnd) == 0){
            // no window to release
            return false;
        }
        auto idx = captured_windows.at(hWnd);
        captured_windows.erase(hWnd);
        captured_windows_ctx[idx].hWnd = nullptr;
        captured_windows_ctx[idx].status = CapturedWindowContext::Status::FREE;
        lock.unlock();
        ::SendMessageW(hWnd, controlMessage, static_cast<DWORD>(ControllMessageDword::end_capture), 0);
        return true;
    };

    bool changeWindowAttribute(HWND hWnd, HWND hWndInsertAfter, int x, int y, int cx, int cy, int alpha){
        LockHolder lock(mutex);
        if (captured_windows.count(hWnd) == 0){
            // specified window has not been captured yet
            return false;
        }
        auto idx = captured_windows.at(hWnd);
        auto &attrs = captured_windows_ctx[idx];
        attrs.hWndInsertAfter = hWndInsertAfter;
        attrs.x = x;
        attrs.y = y;
        attrs.cx = cx;
        attrs.cy = cy;
        attrs.alpha = alpha;
        lock.unlock();
        ::SendMessageW(hWnd, controlMessage, static_cast<DWORD>(ControllMessageDword::change_attribute), 0);
        return true;
    };
};

static std::unique_ptr<LeadManager> leadManager;

//============================================================================================
// Managing captured windows of all processes except fsmapper
//    This class behaves as singleton pateern
//============================================================================================
class FollowingManager;
static std::unique_ptr<FollowingManager> followingManager;

class FollowingManager : public Manager{
protected:
    std::map<HWND, int> capturedWindows;

public:
    HookedApi<LONG_PTR WINAPI (HWND, int, LONG_PTR)> SetWindowLongPtrW;
    HookedApi<BOOL WINAPI (HWND, HWND, int, int, int, int, UINT)> SetWindowPos;


public:
    FollowingManager() = delete;
    FollowingManager(HMODULE hModule) : Manager(false, hModule){
        SetWindowLongPtrW.setHook("User32.dll", "NtUserSetWindowLongPtr", SetWindowLongPtrW_Hook);
        SetWindowPos.setHook("User32.dll", "NtUserSetWindowPos", SetWindowPos_Hook);
    };
    ~FollowingManager() = default;

protected:
    static LONG_PTR WINAPI SetWindowLongPtrW_Hook(HWND hWnd, int nIndex, LONG_PTR dwNewLong){
        return followingManager->SetWindowLongPtrW(hWnd, nIndex, dwNewLong);
    };

    static BOOL WINAPI SetWindowPos_Hook(HWND hWnd, HWND hWndInsertAfter, int x, int y, int cx, int cy, UINT uFlags){
        return followingManager->SetWindowPos(hWnd, hWndInsertAfter, x, y, cx, cy, uFlags);
    };
};

//============================================================================================
// Hook procedure
//============================================================================================
LRESULT CALLBACK hookProc(int nCode, WPARAM wParam, LPARAM lParam){
    if (nCode >= HC_ACTION) {
        auto* pMsg = reinterpret_cast<CWPSTRUCT*>(lParam);
        if (followingManager && pMsg->message == followingManager->getControlMessageCode()) {
            std::ostringstream os;
            os << "msg: " << pMsg->message << " hWnd: "<< pMsg->hwnd << std::endl;
            OutputDebugStringA(os.str().c_str());
        }
    }
    return CallNextHookEx(hookHandle, nCode, wParam, lParam);
}


//============================================================================================
// DLL entry point
//============================================================================================
static HINSTANCE hInstance;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved){
    switch (ul_reason_for_call){
    case DLL_PROCESS_ATTACH:
        hInstance = hModule;
        try{
            if (hookHandle != 0){
                followingManager = std::make_unique<FollowingManager>(hModule);
            }
        }catch (FatalException &e){
            DWORD error = e.error;
            return true;
        }
        break;
    case DLL_PROCESS_DETACH:
        leadManager = nullptr;
        followingManager = nullptr;
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }
    return true;
}

//============================================================================================
// exported functions
//============================================================================================
DLLEXPORT bool hookdll_startGlobalHook(WINDOW_CLOSE_CALLBACK callback, void* context){
    if (hookHandle){
        return false;
    }
    try{
        leadManager = std::make_unique<LeadManager>(hInstance, callback, context);
    }catch (FatalException &e){
        DWORD error = e.error;
        return false;
    }
    return true;
}

DLLEXPORT bool hookdll_stopGlobalHook(){
    leadManager = nullptr;
    return true;
}

DLLEXPORT bool hookdll_capture(HWND hWnd){
    if (leadManager){
        return leadManager->addCapture(hWnd);
    }else{
        return false;
    }
}

DLLEXPORT bool hookdll_uncapture(HWND hWnd){
    if (leadManager){
        return leadManager->releaseCapture(hWnd);
    }else{
        return false;
    }
}

DLLEXPORT bool hookdll_changeWindowAtrribute(HWND hWnd, HWND hWndInsertAfter, int x, int y, int cx, int cy, int alpha){
    if (leadManager){
        return leadManager->changeWindowAttribute(hWnd, hWndInsertAfter, x, y, cx, cy, alpha);
    }else{
        return false;
    }
}