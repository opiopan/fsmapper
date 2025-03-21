//
// mouseemu.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include "mouseemu.h"

#include <thread>
#include <mutex>
#include <condition_variable>
#include <sstream>

using namespace mouse_emu;

static constexpr auto ring_buff_size = 64;
static constexpr auto pointer_position_recovery_time = milliseconds(1000);
static constexpr auto pointer_position_recovery_message_interval = milliseconds(50);

struct command{
    event ev;
    int32_t x;
    int32_t y;
    clock::time_point time;
};

class emulator_imp : public emulator{
    std::mutex mutex;
    std::thread event_sender;
    std::condition_variable cv;
    bool should_stop = false;
    command commands[ring_buff_size];
    int command_next = 0;
    int command_top = 0;
    int primary_screen_x;
    int primary_screen_y;
    bool pointer_is_on_primary_window = true;
    bool need_to_click = false;
    bool in_down_state = false;
    HWND window_for_recovery{0};
    mouse_emu::recovery_type recovery_action{mouse_emu::recovery_type::none};
    DWORD recovery_down{0};
    DWORD recovery_up{0};

public:
    emulator_imp(){
        primary_screen_x = GetSystemMetrics(SM_CXSCREEN);
        primary_screen_y = GetSystemMetrics(SM_CYSCREEN);

        event_sender = std::move(std::thread([this]{
            std::unique_lock lock(mutex);
            while (true){
                while (command_top == command_next){
                    if (!pointer_is_on_primary_window && !in_down_state){
                        if (!cv.wait_for(lock, pointer_position_recovery_time, [this]{return command_next > command_top || should_stop;})){
                            recover_pointer_position(need_to_click);
                            pointer_is_on_primary_window = true;
                            need_to_click = false;
                        }
                    }else{
                        cv.wait(lock, [this]{return command_next > command_top || should_stop;});
                    }
                    if (should_stop){
                        return;
                    }
                }
                auto& command = commands[command_top & (ring_buff_size - 1)];
                if (command.ev == event::recover){
                    command_top++;
                    pointer_is_on_primary_window = false;
                    need_to_click = true;
                    in_down_state = false;
                    continue;
                }else if (command.ev == event::cancel_recovery){
                    command_top++;
                    pointer_is_on_primary_window = true;
                    need_to_click = false;
                    continue;
                }
                lock.unlock();
                
                auto now = clock::now();
                if (now < command.time){
                    std::this_thread::sleep_until(command.time);
                }

                #ifdef _DEBUG
                    static clock::time_point base{clock::now()};
                    std::ostringstream os;
                    auto ref_time = std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - base);
                    os << "issueMouseEvent(" << ref_time;
                    os << "): [" << static_cast<DWORD>(command.ev) << "] x:" << command.x << ", y:" << command.y;
                    os << ", screen.x: " << primary_screen_x << ", screen.y: " << primary_screen_y; 
                    os << std::endl;
                    OutputDebugStringA(os.str().c_str());
                #endif

                INPUT input;
                input.type = INPUT_MOUSE;
                input.mi.dx = (static_cast<int64_t>(command.x) * 65535) / primary_screen_x;
                input.mi.dy = (static_cast<int64_t>(command.y) * 65535) / primary_screen_y;
                input.mi.mouseData = 0;
                input.mi.dwFlags = static_cast<DWORD>(command.ev) | MOUSEEVENTF_ABSOLUTE;
                input.mi.time = 0;
                input.mi.dwExtraInfo = signature;
                ::SendInput(1, &input, sizeof(INPUT));

                lock.lock();
                if (command.ev == event::down){
                    in_down_state = true;
                }else if (command.ev == event::up){
                    in_down_state = false;
                }
                command_top++;
                pointer_is_on_primary_window = false;
                need_to_click = false;
                cv.notify_all();
            }
        }));
    }

    virtual ~emulator_imp(){
        {
            std::lock_guard lock(mutex);
            should_stop = true;
            cv.notify_all();
        }
        event_sender.join();
    }

    void set_window_for_recovery(HWND hwnd, recovery_type type) override{
        std::unique_lock lock(mutex);
        window_for_recovery = hwnd;
        recovery_action = type;
        if (recovery_action == recovery_type::left){
            recovery_down = MOUSEEVENTF_LEFTDOWN;
            recovery_up = MOUSEEVENTF_LEFTUP;
        }else if (recovery_action == recovery_type::right){
            recovery_down = MOUSEEVENTF_RIGHTDOWN;
            recovery_up = MOUSEEVENTF_RIGHTUP;
        }else if (recovery_action == recovery_type::middle){
            recovery_down = MOUSEEVENTF_MIDDLEDOWN;
            recovery_up = MOUSEEVENTF_MIDDLEUP;
        }
    }

    void emulate(event ev, int32_t x, int32_t y, clock::time_point at) override{
        std::unique_lock lock(mutex);
        if (command_next - command_top >= ring_buff_size){
            cv.wait(lock, [this]{return command_next - command_top < ring_buff_size;});
        }
        auto& command = commands[command_next & (ring_buff_size - 1)];
        command.ev = ev;
        command.x = x;
        command.y = y;
        command.time = at;
        command_next++;
        cv.notify_all();
    };

protected:
    void recover_pointer_position(bool with_click = false){
        RECT rect;
        if (::GetWindowRect(window_for_recovery, &rect)){
            auto x = rect.left + (rect.right - rect.left) * 0.5;
            auto y = rect.top + (rect.bottom - rect.top) * 0.9;

            INPUT input;
            input.type = INPUT_MOUSE;
            input.mi.dx = static_cast<LONG>((static_cast<int64_t>(x) * 65535) / primary_screen_x);
            input.mi.dy = static_cast<LONG>((static_cast<int64_t>(y) * 65535) / primary_screen_y);
            input.mi.mouseData = 0;
            input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
            input.mi.time = 0;
            input.mi.dwExtraInfo = recovery_signature;
            ::SendInput(1, &input, sizeof(INPUT));
            
            if (!with_click || recovery_action == recovery_type::none){
                return;
            }

            std::this_thread::sleep_for(pointer_position_recovery_message_interval);
            input.mi.dwFlags = recovery_down | MOUSEEVENTF_ABSOLUTE;
            ::SendInput(1, &input, sizeof(INPUT));

            std::this_thread::sleep_for(pointer_position_recovery_message_interval);
            input.mi.dwFlags = recovery_up | MOUSEEVENTF_ABSOLUTE;
            ::SendInput(1, &input, sizeof(INPUT));
        }
    }
};

std::unique_ptr<emulator> mouse_emu::create_emulator(){
    return std::make_unique<emulator_imp>();
}
