//
// hookdll.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include <memory>
#include <sstream>
#include <map>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <optional>
#include <chrono>

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
    bool show;
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
                            target.hWnd = nullptr;
                            itr = captured_windows.erase(itr);
                            ::update_counter.from_leader++;
                            last_update_counter = ::update_counter;
                            if (this->callback) {
                                lock.unlock();
                                this->callback(hWnd, this->callback_ctx);
                                lock.lock();
                            }
                            if (last_update_counter.from_leader != ::update_counter.from_leader ||
                                last_update_counter.from_follower != ::update_counter.from_follower) {
                                itr = captured_windows.begin();
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

    bool addCapture(HWND hWnd, bool hide_system_region){
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
        update_counter.from_leader++;
        lock.unlock();
        ::SendMessageW(hWnd, controlMessage, static_cast<DWORD>(ControllMessageDword::start_capture), hide_system_region);
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
        update_counter.from_leader++;
        lock.unlock();
        ::SendMessageW(hWnd, controlMessage, static_cast<DWORD>(ControllMessageDword::end_capture), 0);
        return true;
    };

    bool changeWindowAttribute(HWND hWnd, HWND hWndInsertAfter, int x, int y, int cx, int cy, bool show){
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
        attrs.show = show;
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
    struct WindowContext{
        int index;
        HWND hWndInsertAfter;
        int x;
        int y;
        int cx;
        int cy;
        bool show;
        LONG_PTR saved_style;
        RECT saved_rect;
        int change_request_num;
    };
    struct ChangeRequest{
        bool change_position:1;
        bool change_visibility:1;
        HWND hWnd;
        HWND hWndInsertAfter;
        int x;
        int y;
        int cx;
        int cy;
        bool show;
    };
    std::mutex lmutex;
    std::condition_variable cv;
    std::queue<ChangeRequest> queue;
    int64_t local_count = 0;
    bool should_stop = false;
    std::optional<std::thread> attribute_changer;
    std::map<HWND, WindowContext> captured_windows;

    HookedApi<LONG_PTR WINAPI (HWND, int, LONG_PTR)> SetWindowLongPtrW;
    HookedApi<BOOL WINAPI (HWND, HWND, int, int, int, int, UINT)> SetWindowPos;

public:
    FollowingManager() = delete;
    FollowingManager(HMODULE hModule) : Manager(false, hModule){
        SetWindowLongPtrW.setHook("User32.dll", "NtUserSetWindowLongPtr", SetWindowLongPtrW_Hook);
        SetWindowPos.setHook("User32.dll", "NtUserSetWindowPos", SetWindowPos_Hook);
    };
    ~FollowingManager(){
        if (attribute_changer.has_value()){
            std::unique_lock lock(lmutex);
            should_stop = true;
            cv.notify_all();
            lock.unlock();
            attribute_changer.value().join();
        }
        std::unique_lock lock(lmutex);
        auto itr = captured_windows.begin();
        while (itr != captured_windows.end()){
            auto last_count = local_count;
            auto hWnd = itr->first;
            itr++;
            lock.unlock();
            releaseWindow(hWnd);
            lock.lock();
            if (last_count + 1 != local_count){
                itr = captured_windows.begin();
            }
        }
    };

    void captureWindow(HWND hWnd, bool hide_system_region){
        LockHolder glock(mutex);
        {
            std::unique_lock llock(lmutex);
            if (captured_windows.count(hWnd) > 0){
                // already captured
                return;
            }
            auto i = 0;
            for (; i < MAX_CAPTURED_WINDOW; i++){
                if (captured_windows_ctx[i].hWnd == hWnd && 
                    captured_windows_ctx[i].status == CapturedWindowContext::Status::CAPTURED){
                    break;
                }
            }
            if (i < MAX_CAPTURED_WINDOW){
                local_count++;
                WindowContext ctx;
                ctx.index = i;
                ctx.saved_style  = ::GetWindowLongPtrW(hWnd, GWL_STYLE);
                ::GetWindowRect(hWnd, &ctx.saved_rect);
                ctx.show = false;
                ctx.x = 0;
                ctx.y = 0;
                ctx.cx = 0;
                ctx.cy = 0;
                ctx.hWndInsertAfter = nullptr;
                ctx.change_request_num = 0;
                captured_windows.emplace(hWnd, ctx);
                llock.unlock();
                glock.unlock();
                if (hide_system_region){
                    this->SetWindowLongPtrW(
                        hWnd, GWL_STYLE, 
                        ctx.saved_style ^(WS_BORDER | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_DLGFRAME));
                }
                this->SetWindowPos(hWnd, nullptr, 0, 0, 0, 0, SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_HIDEWINDOW);
            }
        }
    };

    void releaseWindow(HWND hWnd){
        std::unique_lock llock(lmutex);
        if (captured_windows.count(hWnd) == 0){
            // specified window is not captured
            return;
        }
        auto& ctx = captured_windows.at(hWnd);
        cv.wait(llock, [&ctx](){return ctx.change_request_num == 0;});
        auto saved_style = ctx.saved_style;
        auto saved_rect = ctx.saved_rect;
        captured_windows.erase(hWnd);
        local_count++;
        if (captured_windows.size() == 0 && attribute_changer.has_value()){
            should_stop = true;
            cv.notify_all();
            auto thread = std::move(attribute_changer);
            attribute_changer.reset();
            llock.unlock();
            thread->join();
            llock.lock();
        }
        llock.unlock();
        this->SetWindowLongPtrW(hWnd, GWL_STYLE, saved_style);
        this->SetWindowPos(
            hWnd, HWND_TOP, 
            saved_rect.left, saved_rect.top, saved_rect.right - saved_rect.left, saved_rect.bottom - saved_rect.top, 
            SWP_FRAMECHANGED | SWP_SHOWWINDOW);
    };

    void changeWindowAttribute(HWND hWnd){
        LockHolder glock(mutex);
        {
            std::unique_lock llock(lmutex);
            if (captured_windows.count(hWnd) == 0){
                // specified window is not captured
                return;
            }
            auto& ctx = captured_windows.at(hWnd);
            auto& req = captured_windows_ctx[ctx.index];
            if (req.hWnd != hWnd || req.status != CapturedWindowContext::Status::CAPTURED){
                return;
            }
            ChangeRequest cr = {0};
            cr.hWnd = hWnd;
            if (req.x != ctx.x || req.y != ctx.y || req.cx != ctx.cx || req.cy != ctx.cy){
                cr.change_position = true;
                ctx.x = cr.x = req.x;
                ctx.y = cr.y = req.y;
                ctx.cx = cr.cx = req.cx;
                ctx.cy = cr.cy = req.cy;
                ctx.hWndInsertAfter = cr.hWndInsertAfter = req.hWndInsertAfter;
            }
            if (req.show != ctx.show){
                cr.change_visibility = true;
                ctx.show = cr.show = req.show;
            }
            if (!cr.change_position && !cr.change_visibility){
                return;
            }

            // change window attributes
            llock.unlock();
            UINT flag = cr.change_position ? 0 : SWP_NOMOVE | SWP_NOSIZE;
            if (cr.change_visibility){
                flag |= req.show ? SWP_SHOWWINDOW : SWP_HIDEWINDOW;
            }
            this->SetWindowPos(cr.hWnd, cr.hWndInsertAfter, cr.x, cr.y, cr.cx, cr.cy, flag);
        }
    };

    void closeWindow(HWND hWnd){
        LockHolder glock(mutex);
        {
            std::unique_lock llock(lmutex);
            if (captured_windows.count(hWnd) == 0){
                // specified window is not captured
                return;
            }
            auto& ctx = captured_windows.at(hWnd);
            auto& gctx = captured_windows_ctx[ctx.index];
            if (gctx.status == CapturedWindowContext::Status::CAPTURED){
                gctx.status = CapturedWindowContext::Status::CLOSED;
                update_counter.from_follower++;
                ::SetEvent(event);
            }
            captured_windows.erase(hWnd);
        }
    };

protected:
    static LONG_PTR WINAPI SetWindowLongPtrW_Hook(HWND hWnd, int nIndex, LONG_PTR dwNewLong){
        std::unique_lock llock(followingManager->lmutex);
        if (nIndex == GWL_STYLE && followingManager->captured_windows.count(hWnd) > 0){
            return followingManager->captured_windows.at(hWnd).saved_style;
        }else{
            llock.unlock();
            return followingManager->SetWindowLongPtrW(hWnd, nIndex, dwNewLong);
        }
    };

    static BOOL WINAPI SetWindowPos_Hook(HWND hWnd, HWND hWndInsertAfter, int x, int y, int cx, int cy, UINT uFlags){
        std::unique_lock llock(followingManager->lmutex);
        if (followingManager->captured_windows.count(hWnd) > 0){
            return true;
        }else{
            llock.unlock();
            return followingManager->SetWindowPos(hWnd, hWndInsertAfter, x, y, cx, cy, uFlags);
        }
    };
};

//============================================================================================
// Hook procedure
//============================================================================================
LRESULT CALLBACK hookProc(int nCode, WPARAM wParam, LPARAM lParam){
    if (nCode >= HC_ACTION) {
        auto* pMsg = reinterpret_cast<CWPSTRUCT*>(lParam);
        if (followingManager){
            if (pMsg->message == followingManager->getControlMessageCode()) {
                auto type = static_cast<ControllMessageDword>(pMsg->wParam);
                if (type == ControllMessageDword::start_capture){
                    followingManager->captureWindow(pMsg->hwnd, pMsg->lParam);
                }else if (type == ControllMessageDword::end_capture){
                    followingManager->releaseWindow(pMsg->hwnd);
                }else if (type == ControllMessageDword::change_attribute){
                    followingManager->changeWindowAttribute(pMsg->hwnd);
                }
            }else if (pMsg->message == WM_DESTROY){
                followingManager->closeWindow(pMsg->hwnd);
            }
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

DLLEXPORT bool hookdll_capture(HWND hWnd, bool hide_system_region){
    if (leadManager){
        return leadManager->addCapture(hWnd, hide_system_region);
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

DLLEXPORT bool hookdll_changeWindowAtrribute(HWND hWnd, HWND hWndInsertAfter, int x, int y, int cx, int cy, bool show){
    if (leadManager){
        return leadManager->changeWindowAttribute(hWnd, hWndInsertAfter, x, y, cx, cy, show);
    }else{
        return false;
    }
}