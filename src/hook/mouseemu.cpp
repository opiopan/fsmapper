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

public:
    emulator_imp(){
        primary_screen_x = GetSystemMetrics(SM_CXSCREEN);
        primary_screen_y = GetSystemMetrics(SM_CYSCREEN);

        event_sender = std::move(std::thread([this]{
            std::unique_lock lock(mutex);
            while (true){
                if (command_top == command_next){
                    cv.wait(lock, [this]{return command_next > command_top || should_stop;});
                }
                if (should_stop){
                    return;
                }
                auto& command = commands[command_top & (ring_buff_size - 1)];
                lock.unlock();
                
                auto now = clock::now();
                if (now < command.time){
                    std::this_thread::sleep_until(command.time);
                }

                // std::ostringstream os;
                // os << "issueMouseEvent(" << command_top;
                // os << "): [" << static_cast<DWORD>(command.ev) << "] x:" << command.x << ", y:" << command.y;
                // os << ", screen.x: " << primary_screen_x << ", screen.y: " << primary_screen_y << std::endl;
                // OutputDebugStringA(os.str().c_str());

                INPUT input;
                input.type = INPUT_MOUSE;
                input.mi.dx = (static_cast<int64_t>(command.x) * 65535) / primary_screen_x;
                input.mi.dy = (static_cast<int64_t>(command.y) * 65535) / primary_screen_y;
                input.mi.mouseData = 0;
                input.mi.dwFlags = static_cast<DWORD>(command.ev) | MOUSEEVENTF_ABSOLUTE;
                input.mi.time = 0;
                input.mi.dwExtraInfo = 0;
                ::SendInput(1, &input, sizeof(INPUT));

                lock.lock();
                command_top++;
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
};

std::unique_ptr<emulator> mouse_emu::create_emulator(){
    return std::make_unique<emulator_imp>();
}
