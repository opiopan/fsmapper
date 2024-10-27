//
// windowcapture.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once

#include <memory>
#include <vector>
#include <string>
#include <sol/sol.hpp>
#include <windows.h>

class ViewPortManager;

namespace capture{
    class image_streamer{
    public:
        virtual uint32_t get_id() const = 0;
        virtual const char* get_name() const = 0;
        virtual HWND get_hwnd() const = 0;
        virtual const std::vector<std::string>& get_target_titles() const = 0;

        virtual void set_hwnd(HWND) = 0;
        virtual void start_capture() = 0;
        virtual void stop_capture() = 0;
        virtual void dispose() = 0;
    };

    std::shared_ptr<image_streamer> create_image_streamer(ViewPortManager& manager, uint32_t id, sol::object arg);

    void init_scripting_env(sol::table& mapper_table);
};
