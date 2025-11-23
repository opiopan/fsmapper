//
// config.hpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once

#include <filesystem>
#include <optional>
#include "mappercore.h"

namespace fsmapper{
    struct rect{
        int32_t left;
        int32_t top;
        int32_t width;
        int32_t height;

        rect() = default;
        rect(const rect&) = default;
        rect(int32_t left, int32_t top, int32_t width, int32_t height) : 
            left(left), top(top), width(width), height(height){};

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
        virtual bool get_developer_log() = 0;
        virtual void set_developer_log(bool value) = 0;
        virtual const char* get_pre_run_script() = 0;
        virtual void set_pre_run_script(const char* value) = 0;
        virtual bool get_pre_run_script_is_valid() = 0;
        virtual void set_pre_run_script_is_valid(bool value) = 0;
        virtual MAPPER_OPTION_RENDERING_METHOD get_rendering_method() = 0;
        virtual void set_rendering_method(MAPPER_OPTION_RENDERING_METHOD value) = 0;
        virtual bool get_use_separated_ui_thread() = 0;
        virtual void set_use_separated_ui_thread(bool value)  = 0;
        virtual bool get_plugin_folder_is_default() = 0;
        virtual void set_plugin_folder_is_default(bool value) = 0;
        virtual const std::filesystem::path &get_default_plugin_folder() = 0;
        virtual const std::filesystem::path &get_custom_plugin_folder() = 0;
        virtual void set_custom_plugin_folder(std::filesystem::path&& path) = 0;
        virtual uint64_t get_lua_standard_libraries() = 0;
        virtual void set_lua_standard_libraries(uint64_t value) = 0;
        virtual const char* get_skipped_version() = 0;
        virtual void set_skipped_version(const char* value) = 0;

        enum class dcs_exporter_mode: int {unknown = 0, on = 1, off = 2};
        virtual dcs_exporter_mode get_dcs_exporter_mode() = 0;
        virtual void set_dcs_exporter_mode(dcs_exporter_mode value) = 0;

        virtual bool get_touch_mouse_emulation_is_enable() = 0;
        virtual void set_touch_mouse_emulation_is_enable(bool value) = 0;
        virtual bool get_touch_delay_mouse_emulation() = 0;
        virtual void set_touch_delay_mouse_emulation(bool value) = 0;
        virtual uint32_t get_touch_down_delay() = 0;
        virtual void set_touch_down_delay(uint32_t) = 0;
        virtual uint32_t get_touch_up_delay() = 0;
        virtual void set_touch_up_delay(uint32_t) = 0;
        virtual uint32_t get_touch_drag_start_delay() = 0;
        virtual void set_touch_drag_start_delay(uint32_t) = 0;
        virtual bool get_touch_double_tap_on_drag() = 0;
        virtual void set_touch_double_tap_on_drag(bool value) = 0;
        virtual uint32_t get_touch_deadzone_for_drag() = 0;
        virtual void set_touch_deadzone_for_drag(uint32_t value) = 0;
        virtual uint32_t get_touch_pointer_jitter() = 0;
        virtual void set_touch_pointer_jitter(uint32_t value) = 0;
        virtual uint32_t get_touch_move_trigger_distance() = 0;
        virtual void set_touch_move_trigger_distance(uint32_t value) = 0;
        virtual uint32_t get_touch_minimum_interval() = 0;
        virtual void set_touch_minimum_interval(uint32_t value) = 0;
        virtual void reset_touch_delay() = 0;
        virtual void reset_touch_delay_for_remote() = 0;

        // For exchanging CLI parameters between processes
        virtual void save_cli_params() = 0;
        virtual void load_cli_params() = 0;
        virtual bool get_cli_launch_minimized() = 0;
        virtual void set_cli_launch_minimized(bool value) = 0;
        virtual const std::optional<std::filesystem::path>& get_cli_script_path() = 0;
        virtual void set_cli_script_path(const wchar_t* value) = 0;
    };

    void init_app_config();
    extern config& app_config;
}
