//
// mouseemu.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once

#include <memory>
#include <windows.h>
#include <chrono>

namespace mouse_emu{
    enum class event{
        down = MOUSEEVENTF_LEFTDOWN,
        up = MOUSEEVENTF_LEFTUP,
        move = MOUSEEVENTF_MOVE,
        recover =-1,
        cancel_recovery = -2,
    };

    using clock = std::chrono::steady_clock;
    using milliseconds = std::chrono::milliseconds;

    class emulator{
    public:
        virtual void emulate(event ev, int32_t x, int32_t y, clock::time_point at) = 0;
    };

    std::unique_ptr<emulator> create_emulator();
}
