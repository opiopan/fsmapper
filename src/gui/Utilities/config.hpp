//
// config.hpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once

#include <filesystem>
#include "mappercore.h"

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
        virtual uint32_t get_message_buffer_size() = 0;
        virtual void set_message_buffer_size(uint32_t value) = 0;
        virtual const char* get_pre_run_script() = 0;
        virtual void set_pre_run_script(const char* value) = 0;
        virtual bool get_pre_run_script_is_valid() = 0;
        virtual void set_pre_run_script_is_valid(bool value) = 0;
        virtual MAPPER_OPTION_RENDERING_METHOD get_rendering_method() = 0;
        virtual void set_rendering_method(MAPPER_OPTION_RENDERING_METHOD value) = 0;
        virtual bool get_plugin_folder_is_default() = 0;
        virtual void set_plugin_folder_is_default(bool value) = 0;
        virtual const std::filesystem::path &get_default_plugin_folder() = 0;
        virtual const std::filesystem::path &get_custom_plugin_folder() = 0;
        virtual void set_custom_plugin_folder(std::filesystem::path&& path) = 0;
    };

    void init_app_config();
    extern config& app_config;
}
