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
#include <algorithm>
using std::max;
using std::min;

#define NO_SOL
#include "tools.h"
#include "apihook.h"
#include "hookdll.h"
#include "mouseemu.h"
#include "hooklog.h"

struct FatalException{
    DWORD error;

    FatalException():error(::GetLastError()){};
    FatalException(const FatalException&) = default;
    FatalException(FatalException&&) = default;
    ~FatalException() = default;
};

static HINSTANCE hInstance;

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
static bool     enable_log{false};
static bool     touch_delay_mouse_emulation{false};
static uint32_t touch_down_delay{0};
static uint32_t touch_up_delay{0};
static uint32_t touch_start_delay{0};
static bool     touch_double_tap_on_drag{false};
static uint32_t touch_dead_zone_for_drag_start{0};
static uint32_t touch_pointer_jitter{0};
static uint32_t touch_move_triger_distance{0};
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
    set_window_for_recovery,
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

    bool addCapture(HWND hWnd, DWORD option){
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
        ::SendMessageW(hWnd, controlMessage, static_cast<DWORD>(ControllMessageDword::start_capture), option);
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

    void setWinodForRecovery(HWND hwnd, int type){
        ::SendMessageW(hwnd, controlMessage, static_cast<DWORD>(ControllMessageDword::set_window_for_recovery), type);
    }
};

static std::unique_ptr<LeadManager> leadManager;

//============================================================================================
// Managing captured windows of all processes except fsmapper
//    This class behaves as singleton pateern
//============================================================================================
LRESULT hookWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

class FollowingManager;
static std::unique_ptr<FollowingManager> sharedFollowingManager;
static thread_local std::unique_ptr<FollowingManager> followingManager;

class FollowingManager : public Manager{
protected:
    struct WindowContext{
        int index;
        HWND hWndInsertAfter;
        WNDPROC original_proc{nullptr};
        int x;
        int y;
        int cx;
        int cy;
        bool show;
        LONG_PTR saved_style;
        RECT saved_rect;
        int change_request_num;
        bool need_to_modify_touch{false};
        bool enable_log{false};
        bool delay_mouse_emulation{false};
        mouse_emu::milliseconds delay_start;
        mouse_emu::milliseconds delay_down;
        mouse_emu::milliseconds delay_up;
        mouse_emu::milliseconds delay_drag;
        int acceptable_delta{5};
        bool double_tap_on_drag{false};
        int dead_zone_for_drag{0};
        int pointer_jitter{1};
        bool pointer_jitter_polarity{true};
        mouse_emu::clock::time_point last_ops_time = mouse_emu::clock::now();
        mouse_emu::clock::time_point last_down_time = mouse_emu::clock::now();
        mouse_emu::clock::time_point last_up_time = mouse_emu::clock::now();
        UINT32 pointer_id{0};
        bool is_delayed_emulation{false};
        bool is_touch_down{false};
        bool is_dragging{false};
        POINT last_raw_down_point{0, 0};
        POINT last_jittered_point{0, 0};
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
    int64_t local_count = 0;
    std::map<HWND, WindowContext> captured_windows;
    std::unique_ptr<mouse_emu::emulator> mouse_emulator;
    HWND window_for_recovery{0};
    mouse_emu::recovery_type recovery_type{mouse_emu::recovery_type::none};

    HookedApi<LONG_PTR WINAPI (HWND, int, LONG_PTR)> SetWindowLongPtrW;
    HookedApi<BOOL WINAPI (HWND, HWND, int, int, int, int, UINT)> SetWindowPos;

public:
    FollowingManager() = delete;
    FollowingManager(HMODULE hModule) : Manager(false, hModule){};
    ~FollowingManager(){
        auto itr = captured_windows.begin();
        while (itr != captured_windows.end()){
            auto last_count = local_count;
            auto hWnd = itr->first;
            itr++;
            releaseWindow(hWnd);
            if (last_count + 1 != local_count){
                itr = captured_windows.begin();
            }
        }
    };

    void captureWindow(HWND hWnd, DWORD option){
        LockHolder glock(mutex);
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
            sharedFollowingManager->SetWindowLongPtrW.setHook("User32.dll", "NtUserSetWindowLongPtr", SetWindowLongPtrW_Hook);
            sharedFollowingManager->SetWindowPos.setHook("User32.dll", "NtUserSetWindowPos", SetWindowPos_Hook);
            local_count++;
            WindowContext ctx;
            ctx.index = i;
            ctx.original_proc = reinterpret_cast<WNDPROC>(::GetWindowLongPtrW(hWnd, GWLP_WNDPROC));
            sharedFollowingManager->SetWindowLongPtrW(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(hookWndProc));
            ctx.saved_style  = ::GetWindowLongPtrW(hWnd, GWL_STYLE);
            ::GetWindowRect(hWnd, &ctx.saved_rect);
            ctx.show = false;
            ctx.x = 0;
            ctx.y = 0;
            ctx.cx = 0;
            ctx.cy = 0;
            ctx.hWndInsertAfter = nullptr;
            ctx.change_request_num = 0;
            ctx.enable_log = enable_log;
            if (option & CAPTURE_OPT_MODIFY_TOUCH && !IsTouchWindow(hWnd, nullptr)) {
                ctx.delay_mouse_emulation = touch_delay_mouse_emulation;
                ctx.need_to_modify_touch = true;
                ctx.delay_start = mouse_emu::milliseconds{touch_start_delay};
                ctx.delay_down = mouse_emu::milliseconds{touch_down_delay};
                ctx.delay_up = mouse_emu::milliseconds{touch_up_delay};
                ctx.delay_drag = mouse_emu::milliseconds{0};
                ctx.double_tap_on_drag = touch_double_tap_on_drag;
                ctx.dead_zone_for_drag = touch_dead_zone_for_drag_start;
                ctx.pointer_jitter = static_cast<int>(touch_pointer_jitter);
                ctx.acceptable_delta = static_cast<int>(touch_move_triger_distance);
                RegisterTouchWindow(hWnd, 0);
            }
            if (ctx.need_to_modify_touch && !mouse_emulator){
                mouse_emulator = std::move(mouse_emu::create_emulator());
                mouse_emulator->set_window_for_recovery(window_for_recovery, recovery_type);
            }
            captured_windows.emplace(hWnd, ctx);
            glock.unlock();
            if (option & CAPTURE_OPT_HIDE_SYSTEM_REGION){
                sharedFollowingManager->SetWindowLongPtrW(
                    hWnd, GWL_STYLE, 
                    ctx.saved_style ^(WS_BORDER | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_DLGFRAME));
            }
            sharedFollowingManager->SetWindowPos(hWnd, nullptr, 0, 0, 0, 0, SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_HIDEWINDOW);

            if (ctx.enable_log){
                hooklog::allocate_logger();
                hooklog::get_logger().log(std::format("Captured window: HWND=0x{:X}, option=0x{:X}", reinterpret_cast<uintptr_t>(hWnd), option));
            }
        }
    };

    void releaseWindow(HWND hWnd){
        if (captured_windows.count(hWnd) == 0){
            //specified window is not captured
            return;
        }
        auto& ctx = captured_windows.at(hWnd);
        auto original_proc = ctx.original_proc;
        auto saved_style = ctx.saved_style;
        auto saved_rect = ctx.saved_rect;
        captured_windows.erase(hWnd);
        local_count++;
        if (ctx.need_to_modify_touch){
            UnregisterTouchWindow(hWnd);
        }
        sharedFollowingManager->SetWindowLongPtrW(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(original_proc));
        sharedFollowingManager->SetWindowLongPtrW(hWnd, GWL_STYLE, saved_style);
        sharedFollowingManager->SetWindowPos(
            hWnd, HWND_TOP, 
            saved_rect.left, saved_rect.top, saved_rect.right - saved_rect.left, saved_rect.bottom - saved_rect.top, 
            SWP_FRAMECHANGED | SWP_SHOWWINDOW);
        if (captured_windows.size() == 0){
            mouse_emulator = nullptr;
        }

        if (ctx.enable_log){
            hooklog::get_logger().log(std::format("Released window: HWND=0x{:X}", reinterpret_cast<uintptr_t>(hWnd)));
            hooklog::release_logger();
        }
    };

    void changeWindowAttribute(HWND hWnd){
        LockHolder glock(mutex);
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
        UINT flag = cr.change_position ? 0 : SWP_NOMOVE | SWP_NOSIZE;
        if (cr.change_visibility){
            flag |= req.show ? SWP_SHOWWINDOW : SWP_HIDEWINDOW;
        }
        sharedFollowingManager->SetWindowPos(cr.hWnd, cr.hWndInsertAfter, cr.x, cr.y, cr.cx, cr.cy, flag);
    };

    void closeWindow(HWND hWnd){
        LockHolder glock(mutex);
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
        if (ctx.enable_log){
            hooklog::get_logger().log(std::format("Closed window: HWND=0x{:X}", reinterpret_cast<uintptr_t>(hWnd)));
            hooklog::release_logger();
        }
        captured_windows.erase(hWnd);
    };

    LRESULT callOriginalWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam){
        if (captured_windows.count(hwnd)){
            auto& ctx = captured_windows.at(hwnd);
            return ctx.original_proc(hwnd, msg, wparam, lparam);
        }else{
            return 0;
        }
    }

    bool processTouchMessage(HWND hWnd, UINT msg, WPARAM wparam, LPARAM lparam){
        if (captured_windows.count(hWnd) == 0){
            // specified window is not captured
            return false;
        }
        auto& ctx = captured_windows.at(hWnd);
        if (!ctx.need_to_modify_touch){
            // unnecessary to generate touch message
            return false;
        }
        auto now = mouse_emu::clock::now();
        auto msg_pointer_id = GET_POINTERID_WPARAM(wparam);
        POINTER_INPUT_TYPE pointer_type{PT_POINTER};
        ::GetPointerType(msg_pointer_id, &pointer_type);
        if (pointer_type == PT_MOUSE || (msg != WM_POINTERDOWN && ctx.pointer_id != msg_pointer_id)){
            // not touch message or not relevant pointer id
            return false;
        }
        POINTER_INFO pointer_info;
        ::GetPointerInfo(msg_pointer_id, &pointer_info);
        auto& pt = pointer_info.ptPixelLocation;

        auto process_down = [this, msg_pointer_id, now](WindowContext& ctx, POINT& pt){
            POINT current_point;
            ::GetCursorPos(&current_point);
            auto delta_x = current_point.x - pt.x;
            auto delta_y = current_point.y - pt.y;
            if (delta_x < -ctx.acceptable_delta || delta_x > ctx.acceptable_delta ||
                delta_y < -ctx.acceptable_delta || delta_y > ctx.acceptable_delta){
                ctx.last_ops_time = max(now + ctx.delay_start, ctx.last_ops_time + ctx.delay_start);
                mouse_emulator->emulate(mouse_emu::event::move, pt.x, pt.y, ctx.last_ops_time);
            }
            ctx.last_ops_time = max(now + ctx.delay_down,  max(ctx.last_ops_time + ctx.delay_down, ctx.last_up_time + ctx.delay_down));
            ctx.last_down_time = ctx.last_ops_time;
            mouse_emulator->emulate(mouse_emu::event::down, pt.x, pt.y, ctx.last_ops_time);

            ctx.is_delayed_emulation = false;
        };

        if (msg == WM_POINTERDOWN && !ctx.is_touch_down){
            hooklog::get_logger().log(std::format("Pointer down: id={}, x={}, y={}", msg_pointer_id, pt.x, pt.y));
            ctx.last_raw_down_point = pt;
            auto jitter_delta_x = abs(pt.x - ctx.last_jittered_point.x);
            auto jitter_delta_y = abs(pt.y - ctx.last_jittered_point.y);
            if (jitter_delta_x <= ctx.pointer_jitter && jitter_delta_y <= ctx.pointer_jitter){
                if (ctx.pointer_jitter_polarity){
                    pt.x += ctx.pointer_jitter;
                    pt.y += ctx.pointer_jitter;
                }else{
                    pt.x -= ctx.pointer_jitter;
                    pt.y -= ctx.pointer_jitter;
                }
                ctx.pointer_jitter_polarity = !ctx.pointer_jitter_polarity;
            }
            ctx.last_jittered_point = pt;
            if (ctx.delay_mouse_emulation){
                ctx.is_delayed_emulation = true;
            }else{
                process_down(ctx, pt);
            }
            ctx.pointer_id = msg_pointer_id;
            ctx.is_touch_down = true;
            return true;
        }else if (msg == WM_POINTERUP && ctx.is_touch_down){
            if (ctx.is_delayed_emulation){
                process_down(ctx, ctx.last_jittered_point);
            }
            ctx.pointer_id = 0;
            ctx.is_touch_down = false;
            ctx.is_dragging = false;
            ctx.last_ops_time = max(now, max(ctx.last_ops_time, ctx.last_down_time + ctx.delay_up));
            ctx.last_up_time = ctx.last_ops_time;
            auto delta_x = abs(ctx.last_raw_down_point.x - pt.x);
            auto delta_y = abs(ctx.last_raw_down_point.y - pt.y);
            if (delta_x <= ctx.dead_zone_for_drag && delta_y <= ctx.dead_zone_for_drag){
                pt = ctx.last_raw_down_point;
            }
            mouse_emulator->emulate(mouse_emu::event::up, pt.x, pt.y, ctx.last_ops_time);
            return true;
        }else if (msg == WM_POINTERUPDATE && ctx.is_touch_down){
            if (!ctx.is_dragging){
                auto delta_x = abs(ctx.last_raw_down_point.x - pt.x);
                auto delta_y = abs(ctx.last_raw_down_point.y - pt.y);
                if (delta_x <= ctx.dead_zone_for_drag && delta_y <= ctx.dead_zone_for_drag){
                    return true;
                }
                if (ctx.is_delayed_emulation){
                    process_down(ctx, ctx.last_jittered_point);
                }
                ctx.is_dragging = true;
                if (ctx.double_tap_on_drag){
                    ctx.last_ops_time = max(now, max(ctx.last_ops_time, ctx.last_down_time + ctx.delay_up));
                    ctx.last_up_time = ctx.last_ops_time;
                    mouse_emulator->emulate(mouse_emu::event::up, ctx.last_jittered_point.x, ctx.last_jittered_point.y, ctx.last_ops_time);
                    ctx.last_ops_time = max(now, max(ctx.last_ops_time, ctx.last_up_time + ctx.delay_down));
                    ctx.last_down_time = ctx.last_ops_time;
                    mouse_emulator->emulate(mouse_emu::event::down, ctx.last_jittered_point.x, ctx.last_jittered_point.y, ctx.last_ops_time);
                }
            }
            ctx.last_ops_time = max(now, max(ctx.last_ops_time, ctx.last_down_time + ctx.delay_drag));
            mouse_emulator->emulate(mouse_emu::event::move, pt.x, pt.y, ctx.last_ops_time);
            return true;
        }

        return false;
    }

    void setWindowForRecovery(HWND hwnd, mouse_emu::recovery_type type){
        window_for_recovery = hwnd;
        recovery_type = type;
        if (mouse_emulator){
            mouse_emulator->set_window_for_recovery(window_for_recovery, recovery_type);
        }
    }

protected:
    static LONG_PTR WINAPI SetWindowLongPtrW_Hook(HWND hWnd, int nIndex, LONG_PTR dwNewLong){
        if (nIndex == GWL_STYLE && followingManager && followingManager->captured_windows.count(hWnd) > 0){
            return followingManager->captured_windows.at(hWnd).saved_style;
        }else{
            return sharedFollowingManager->SetWindowLongPtrW(hWnd, nIndex, dwNewLong);
        }
    };

    static BOOL WINAPI SetWindowPos_Hook(HWND hWnd, HWND hWndInsertAfter, int x, int y, int cx, int cy, UINT uFlags){
        if (followingManager && followingManager->captured_windows.count(hWnd) > 0){
            return true;
        }else{
            return sharedFollowingManager->SetWindowPos(hWnd, hWndInsertAfter, x, y, cx, cy, uFlags);
        }
    };
};

//============================================================================================
// Hook procedure
//============================================================================================
LRESULT CALLBACK hookProc(int nCode, WPARAM wParam, LPARAM lParam){
    if (nCode >= HC_ACTION) {
        auto* pMsg = reinterpret_cast<CWPSTRUCT*>(lParam);
        if (sharedFollowingManager){
            if (pMsg->message == sharedFollowingManager->getControlMessageCode()) {
                if (!followingManager){
                    followingManager = std::make_unique<FollowingManager>(hInstance);
                }
                auto type = static_cast<ControllMessageDword>(pMsg->wParam);
                if (type == ControllMessageDword::start_capture){
                    followingManager->captureWindow(pMsg->hwnd, static_cast<DWORD>(pMsg->lParam));
                }else if (type == ControllMessageDword::end_capture){
                    followingManager->releaseWindow(pMsg->hwnd);
                }else if (type == ControllMessageDword::change_attribute){
                    followingManager->changeWindowAttribute(pMsg->hwnd);
                }else if (type == ControllMessageDword::set_window_for_recovery){
                    followingManager->setWindowForRecovery(pMsg->hwnd, static_cast<mouse_emu::recovery_type>(pMsg->lParam));
                }
            }
        }
    }
    return CallNextHookEx(hookHandle, nCode, wParam, lParam);
}

LRESULT hookWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam){
    if (msg == WM_POINTERDOWN || msg == WM_POINTERUP || msg == WM_POINTERUPDATE){
        if (followingManager->processTouchMessage(hwnd, msg, wparam, lparam)){
            return 0;
        }
    }else if (msg == WM_DESTROY){
        followingManager->closeWindow(hwnd);
    }
    
    return followingManager->callOriginalWindowProc(hwnd, msg, wparam, lparam);
}

//============================================================================================
// DLL entry point
//============================================================================================
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved){
    switch (ul_reason_for_call){
    case DLL_PROCESS_ATTACH:
        hInstance = hModule;
        try{
            if (hookHandle != 0){
                sharedFollowingManager = std::make_unique<FollowingManager>(hModule);
            }
        }catch (FatalException &e){
            DWORD error = e.error;
            return true;
        }
        break;
    case DLL_PROCESS_DETACH:
        leadManager = nullptr;
        sharedFollowingManager = nullptr;
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

DLLEXPORT bool hookdll_capture(HWND hWnd, DWORD option){
    if (leadManager){
        return leadManager->addCapture(hWnd, option);
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

DLLEXPORT void hookdll_setWindowForRecovery(HWND hwnd, int type){
    if (leadManager){
        leadManager->setWinodForRecovery(hwnd, type);
    }
}

DLLEXPORT void hookdll_setTouchParameters(const TOUCH_CONFIG* config){
    touch_delay_mouse_emulation = config->delay_mouse_emulation;
    touch_down_delay = config->down_delay;
    touch_up_delay = config->up_delay;
    touch_start_delay = config->start_delay;
    touch_double_tap_on_drag = config->double_tap_on_drag;
    touch_dead_zone_for_drag_start = config->dead_zone_for_drag_start;
    touch_pointer_jitter = config->pointer_jitter;
    touch_move_triger_distance = config->move_trigger_distance;
}

DLLEXPORT void hookdll_setLogMode(bool enable){
    enable_log = enable;
}