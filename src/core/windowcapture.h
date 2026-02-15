//
// windowcapture.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once

#include <memory>
#include <vector>
#include <string>
#include <sol/sol.hpp>
#define NOMINMAX
#include <windows.h>
#include <dcomp.h>

#include "tools.h"

class ViewPortManager;
class ViewPort;
namespace composition{class viewport_target;}

namespace capture{
    class captured_image{
    public:
        virtual void associate_viewport(ViewPort* viewport, composition::viewport_target& target) = 0;
        virtual void set_bounds(const FloatRect& bounds) = 0;
        virtual IDCompositionVisual* get_visual() = 0;
        virtual void dispose() = 0;
    };

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

        virtual std::shared_ptr<captured_image> create_view_element(sol::object arg_object) = 0;
    };

    std::shared_ptr<image_streamer> create_image_streamer(ViewPortManager& manager, uint32_t id, sol::variadic_args args);

    void init_scripting_env(sol::table& mapper_table);
};
