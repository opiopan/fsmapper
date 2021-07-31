//
// caputuredwindow.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include <memory>
#include <sstream>
#include <map>
#include <thread>
#include <optional>

#define NO_SOL
#include "tools.h"
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
        FREE,
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

#pragma comment( linker, "/SECTION:.shared,RWS" )
#pragma data_seg( ".shared" )
static HHOOK hookHandle = 0;
static CapturedWindowContext capturedWindows[MAX_CAPTURED_WINDOW] = {0};
#pragma data_seg()

static HMODULE moduleInstance;

//============================================================================================
// Managing captured windows class definition
//    This class behaves as singleton pateern
//============================================================================================
class Manager {
protected:
    bool isOrigin;
    HMODULE hModule;
    WinHandle mutex;
    WinHandle event;
    UINT controlMessage;
    std::map<HWND, int> capturedWindows;

public:
    Manager() = delete;
    Manager(const Manager&) = delete;
    Manager(Manager&&) = delete;

    Manager(bool isOrigin, HMODULE hModule) : isOrigin(isOrigin), hModule(hModule){
        auto assert = [](bool condition){if (!condition){throw FatalException();}};
        static wchar_t nameBuf[MAX_PATH + 1];
        auto namelen = ::GetModuleFileNameW(hModule, nameBuf, sizeof(nameBuf) / sizeof(*nameBuf));
        assert(namelen != 0);
        for (auto c = nameBuf; *c; c++){
            if (*c == L'\\'){
                *c = L'_';
            }
        }
        std::wstring moduleName(nameBuf);
        if (isOrigin){
            mutex = ::CreateMutexW(nullptr, false, (moduleName + L"mutex").c_str());
            assert(mutex);
            event = ::CreateEventW(nullptr, true, false, (moduleName + L"event").c_str());
            assert(event);
        }else{
            mutex = ::OpenMutexW( MUTEX_ALL_ACCESS, false, (moduleName + L"mutex").c_str());
            assert(mutex);
            event = ::OpenEventW(EVENT_ALL_ACCESS, false, (moduleName + L"event").c_str());
            assert(event);
        }
        controlMessage = ::RegisterWindowMessageW(moduleName.c_str());
        assert(controlMessage);
    }

    ~Manager(){
        //::WaitForSingleObject(mutex, INFINITE);
        //::ReleaseMutex(mutex);
    }

    UINT getControlMessageCode() const {
        return controlMessage;
    };
    HMODULE getHmodule() const {return hModule;};
};

static std::unique_ptr<Manager> manager;

//============================================================================================
// Hook procedure
//============================================================================================
LRESULT CALLBACK hookProc(int nCode, WPARAM wParam, LPARAM lParam){
    if (nCode >= HC_ACTION) {
        auto* pMsg = reinterpret_cast<CWPSTRUCT*>(lParam);
        if (manager && pMsg->message == manager->getControlMessageCode()) {
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
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved){
    switch (ul_reason_for_call){
    case DLL_PROCESS_ATTACH:
        try{
            manager = std::make_unique<Manager>(hookHandle == 0, hModule);
            moduleInstance = hModule;
        }catch (FatalException &e){
            DWORD error = e.error;
            return true;
        }
        break;
    case DLL_PROCESS_DETACH:
        manager = nullptr;
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
    ::hookHandle = ::SetWindowsHookExW(WH_CALLWNDPROC, hookProc, moduleInstance, 0);
    return ::hookHandle;
}

DLLEXPORT bool hookdll_stopGlobalHook(){
    if (hookHandle){
        ::UnhookWindowsHookEx(hookHandle);
        ::hookHandle = 0;
        return true;
    }else{
        return false;
    }
}

DLLEXPORT bool hookdll_capture(HWND hWnd){
    ::SendMessageW(hWnd, manager->getControlMessageCode(), 0, 0);
    return true;
}

DLLEXPORT bool hookdll_uncapture(HWND hWnd){
    return false;
}

DLLEXPORT bool hookdll_changeWindowAtrribute(HWND hWnd, HWND hWndInsertAfter, int x, int y, int cx, int cy, int alpha){
    return false;
}