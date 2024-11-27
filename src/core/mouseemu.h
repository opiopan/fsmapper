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

    enum class recovery_type{
        none,
        left,
        right,
        middle,
    };

    using clock = std::chrono::steady_clock;
    using milliseconds = std::chrono::milliseconds;

    class emulator{
    public:
        virtual void set_window_for_recovery(HWND hwnd, recovery_type type) = 0;
        virtual void emulate(event ev, int32_t x, int32_t y, clock::time_point at) = 0;
    };

    std::unique_ptr<emulator> create_emulator();

    static constexpr DWORD signature_mask = 0xffffff00;
    static constexpr DWORD signature = 0xee313400;
    static constexpr DWORD recovery_signature = 0xee313200;
}
