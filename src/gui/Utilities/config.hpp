//
// config.hpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once

#include <filesystem>

namespace fsmapper{
    struct rect{
        int32_t left;
        int32_t top;
        int32_t width;
        int32_t height;

        rect() = default;
        rect(const rect&) = default;

        bool operator == (const rect& rval){
            return left == rval.left && top == rval.top && 
                   width == rval.width && height == rval.height;
        }

        bool operator != (const rect& rval){
            return !(*this == rval);
        }
    };

    class config{
    public:
        virtual void save() = 0;

        virtual const rect& get_window_rect() = 0;
        virtual void set_window_rect(const rect& rect) = 0;
        virtual const std::filesystem::path& get_script_path() = 0;
        virtual void set_script_path(std::filesystem::path&& path) = 0;
        virtual bool get_is_starting_script_at_start_up() = 0;
        virtual void set_is_starting_script_at_start_up(bool value) = 0;
        virtual int32_t get_message_buffer_size() = 0;
        virtual void set_message_buffer_size(int32_t value) = 0;
    };

    void init_app_config();
    extern config& app_config;
}
