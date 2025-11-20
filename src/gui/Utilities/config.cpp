//
// config.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include "pch.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include "config.hpp"
#include "appfs.hpp"

static const auto* CONFIG_WINDOW_TOP = "window_top";
static const auto* CONFIG_WINDOW_LEFT = "window_left";
static const auto* CONFIG_WINDOW_WIDTH = "window_width";
static const auto* CONFIG_WINDOW_HEIGHT = "window_height";
static const auto* CONFIG_SCRIPT_PATH = "script_path";
static const auto* CONFIG_STARTING_SCRIPT_AT_START = "starting_script_at_start";
static const auto* CONFIG_MESSAGE_BUFFER_SIZE = "message_buffer_size";
static const auto* CONFIG_DEVELOPER_LOG = "developer_log";
static const auto* CONFIG_PRE_RUN_SCRIPT = "pre_run_script";
static const auto* CONFIG_PRE_RUN_SCRIPT_IS_VALID = "pre_run_script_is_valid";
static const auto* CONFIG_RENDERING_METHOD = "rendering_method";
static const auto* CONFIG_USE_SEPARATED_UI_THREAD = "use_separated_ui_thread";
static const auto* CONFIG_PLUGIN_FOLDER_IS_DEFAULT = "plugin_folder_is_default";
static const auto* CONFIG_CUSTOM_PLUGIN_FOLDER = "custom_plugin_folder";
static const auto* CONFIG_LUA_STANDARD_LIBRARIES = "lua_stdlib";
static const auto* CONFIG_SKIPPED_VERSION = "skipped_version";
static const auto* CONFIG_DCS_EXPORTER_MODE = "dcs_exporter_mode";
static const auto* CONFIG_TOUCH_MOUSE_EMULATION = "touch_mouse_emulation";
static const auto* CONFIG_TOUCH_DOWN_DELAY = "touch_down_delay";
static const auto* CONFIG_TOUCH_UP_DELAY = "touch_up_delay";
static const auto* CONFIG_TOUCH_DRAG_START_DELAY = "touch_drag_start_delay";
static const auto* CONFIG_TOUCH_DOUBLE_TAP_ON_DRAG = "touch_double_tap_on_drag";
static const auto* CONFIG_TOUCH_DEADZONE_FOR_DRAG = "touch_deadzone_for_drag";
static const auto* CONFIG_TOUCH_POINTER_JITTER = "touch_pointer_jitter";
static const auto* CONFIG_TOUCH_MOVE_TRIGGER_DISTANCE = "touch_move_trigger_distance";
static const auto* CONFIG_CLI_LAUNCH_MINIMIZED = "cli_launch_minimized";
static const auto* CONFIG_CLI_SCRIPT_PATH = "cli_script_path";

static constexpr uint32_t default_touch_down_delay = 50;
static constexpr uint32_t default_touch_up_delay = 50;
static constexpr uint32_t default_touch_drag_start_delay = 80;
static constexpr bool default_touch_double_tap_on_drag = false;
static constexpr uint32_t default_touch_deadzone_for_drag = 0;
static constexpr uint32_t default_touch_pointer_jitter = 3;
static constexpr uint32_t default_touch_move_trigger_distance = 0;

using namespace fsmapper;
using namespace nlohmann;

class config_imp : public config{
    bool is_dirty = false;

    utils::appfs fs{L"fsmapper"};
    std::filesystem::path config_path;
    std::filesystem::path cli_params_path;

    rect window_rect = {0, 0, -1, -1};
    std::filesystem::path script_path;
    bool is_starting_script_at_start_up{true};
    uint32_t message_buffer_size{300};
    bool developer_log{false};
    std::string pre_run_script;
    bool pre_run_script_is_valid{true};
    MAPPER_OPTION_RENDERING_METHOD rendering_method{MOPT_RENDERING_METHOD_CPU};
    bool use_separated_ui_thread{true};
    bool plugin_folder_is_default{true};
    std::filesystem::path custom_plugin_folder;
    uint64_t lua_standard_libraries{MOPT_STDLIB_BASE | MOPT_STDLIB_PACKAGE | MOPT_STDLIB_MATH | MOPT_STDLIB_TABLE | MOPT_STDLIB_STRING};
    std::string skipped_version{"0.0"};
    dcs_exporter_mode dcs_exporter_mode_data{dcs_exporter_mode::unknown};
    bool touch_mouse_emulation{true};
    uint32_t touch_down_delay{default_touch_down_delay};
    uint32_t touch_up_delay{default_touch_up_delay};
    uint32_t touch_drag_start_delay{default_touch_drag_start_delay};
    bool touch_double_tap_on_drag{default_touch_double_tap_on_drag};
    uint32_t touch_deadzone_for_drag{default_touch_deadzone_for_drag};
    uint32_t touch_pointer_jitter{default_touch_pointer_jitter};
    uint32_t touch_move_trigger_distance{default_touch_move_trigger_distance};

    bool cli_launch_minimized{false};
    std::optional<std::filesystem::path> cli_script_path{std::nullopt};

    template <typename KEY, typename VALUE>
    void reflect_number(json& jobj, const KEY& key, VALUE& var){
        if (jobj.count(key)){
            auto vobj =jobj[key];
            if (vobj.is_number()){
                var = vobj.get<VALUE>();
            }
        }
    }

    template <typename KEY, typename VALUE>
    void reflect_bool(json& jobj, const KEY& key, VALUE& var){
        if (jobj.count(key)){
            auto vobj =jobj[key];
            if (vobj.is_boolean()){
                var = vobj.get<VALUE>();
            }
        }
    }

    template <typename KEY, typename VALUE>
    void reflect_string(json& jobj, const KEY& key, VALUE& var){
        if (jobj.count(key)){
            auto vobj =jobj[key];
            if (vobj.is_string()){
                var = std::move(vobj.get<VALUE>());
            }
        }
    }

    template <typename T>
    void update_value(T& variable, const T& value){
        if (variable != value){
            variable = value;
            is_dirty = true;
        }
    }

    template <typename T>
    void update_value(T& variable, T&& value){
        if (variable != value){
            variable = std::move(value);
            is_dirty = true;
        }
    }

    template <typename T, typename VT>
    void update_value(T& variable, const VT* value){
        if (variable != value){
            variable = value;
            is_dirty = true;
        }
    }

public:
    config_imp(){
        config_path = std::move(fs.get_appdata_path() / L"config.json");
        cli_params_path = std::move(fs.get_appdata_path() / L"cli_params.json");
    };

    void load(){
        if (!std::filesystem::exists(config_path)){
            is_dirty = true;
            save();
        }
        std::ifstream ifs(config_path.string());
        json data;
        ifs >> data;
        reflect_number(data, CONFIG_WINDOW_LEFT, window_rect.left);
        reflect_number(data, CONFIG_WINDOW_TOP, window_rect.top);
        reflect_number(data, CONFIG_WINDOW_WIDTH, window_rect.width);
        reflect_number(data, CONFIG_WINDOW_HEIGHT, window_rect.height);
        std::string path;
        reflect_string(data, CONFIG_SCRIPT_PATH, path);
        script_path = path;
        reflect_bool(data, CONFIG_STARTING_SCRIPT_AT_START, is_starting_script_at_start_up);
        reflect_number(data, CONFIG_MESSAGE_BUFFER_SIZE, message_buffer_size);
        reflect_bool(data, CONFIG_DEVELOPER_LOG, developer_log);
        reflect_string(data, CONFIG_PRE_RUN_SCRIPT, pre_run_script);
        reflect_bool(data, CONFIG_PRE_RUN_SCRIPT_IS_VALID, pre_run_script_is_valid);
        reflect_number(data, CONFIG_RENDERING_METHOD, rendering_method);
        reflect_bool(data, CONFIG_USE_SEPARATED_UI_THREAD, use_separated_ui_thread);
        reflect_bool(data, CONFIG_PLUGIN_FOLDER_IS_DEFAULT, plugin_folder_is_default);
        path.clear();
        reflect_string(data, CONFIG_CUSTOM_PLUGIN_FOLDER, path);
        custom_plugin_folder = path;
        reflect_number(data, CONFIG_LUA_STANDARD_LIBRARIES, lua_standard_libraries);
        reflect_string(data, CONFIG_SKIPPED_VERSION, skipped_version);
        int dcs_exporter_mode_num{static_cast<int>(dcs_exporter_mode::unknown)};
        reflect_number(data, CONFIG_DCS_EXPORTER_MODE, dcs_exporter_mode_num);
        dcs_exporter_mode_data = static_cast<dcs_exporter_mode>(dcs_exporter_mode_num);
        reflect_bool(data, CONFIG_TOUCH_MOUSE_EMULATION, touch_mouse_emulation);
        reflect_number(data, CONFIG_TOUCH_DOWN_DELAY, touch_down_delay);
        reflect_number(data, CONFIG_TOUCH_UP_DELAY, touch_up_delay);
        reflect_number(data, CONFIG_TOUCH_DRAG_START_DELAY, touch_drag_start_delay);
        reflect_bool(data, CONFIG_TOUCH_DOUBLE_TAP_ON_DRAG, touch_double_tap_on_drag);
        reflect_number(data, CONFIG_TOUCH_DEADZONE_FOR_DRAG, touch_deadzone_for_drag);
        reflect_number(data, CONFIG_TOUCH_POINTER_JITTER, touch_pointer_jitter);
        reflect_number(data, CONFIG_TOUCH_MOVE_TRIGGER_DISTANCE, touch_move_trigger_distance);
    }

    void save() override{
        if (is_dirty){
            json data{
                {CONFIG_WINDOW_LEFT, window_rect.left},
                {CONFIG_WINDOW_TOP, window_rect.top},
                {CONFIG_WINDOW_WIDTH, window_rect.width},
                {CONFIG_WINDOW_HEIGHT, window_rect.height},
                {CONFIG_SCRIPT_PATH, script_path.string()},
                {CONFIG_STARTING_SCRIPT_AT_START, is_starting_script_at_start_up},
                {CONFIG_MESSAGE_BUFFER_SIZE, message_buffer_size},
                {CONFIG_DEVELOPER_LOG, developer_log},
                {CONFIG_PRE_RUN_SCRIPT, pre_run_script},
                {CONFIG_PRE_RUN_SCRIPT_IS_VALID, pre_run_script_is_valid},
                {CONFIG_RENDERING_METHOD, rendering_method},
                {CONFIG_USE_SEPARATED_UI_THREAD, use_separated_ui_thread},
                {CONFIG_PLUGIN_FOLDER_IS_DEFAULT, plugin_folder_is_default},
                {CONFIG_CUSTOM_PLUGIN_FOLDER, custom_plugin_folder.string()},
                {CONFIG_LUA_STANDARD_LIBRARIES, lua_standard_libraries},
                {CONFIG_SKIPPED_VERSION, skipped_version},
                {CONFIG_DCS_EXPORTER_MODE, static_cast<int>(dcs_exporter_mode_data)},
                {CONFIG_TOUCH_MOUSE_EMULATION, touch_mouse_emulation},
                {CONFIG_TOUCH_DOWN_DELAY, touch_down_delay},
                {CONFIG_TOUCH_UP_DELAY, touch_up_delay},
                {CONFIG_TOUCH_DRAG_START_DELAY, touch_drag_start_delay},
                {CONFIG_TOUCH_DOUBLE_TAP_ON_DRAG, touch_double_tap_on_drag},
                {CONFIG_TOUCH_DEADZONE_FOR_DRAG, touch_deadzone_for_drag},
                {CONFIG_TOUCH_POINTER_JITTER, touch_pointer_jitter},
                {CONFIG_TOUCH_MOVE_TRIGGER_DISTANCE, touch_move_trigger_distance},
            };
            std::ofstream os(config_path.string());
            os << data;
            os.close();
            is_dirty = false;
        }
    }

    const rect& get_window_rect() override{
        return window_rect;
    }
    void set_window_rect(const rect& rect) override{
        update_value(window_rect, rect);
    }
    const std::filesystem::path& get_script_path() override{
        return script_path;
    }
    void set_script_path(std::filesystem::path&& path) override{
        update_value(script_path, std::move(path));
    }
    bool get_is_starting_script_at_start_up() override{
        return is_starting_script_at_start_up;
    };
    void set_is_starting_script_at_start_up(bool value) override{
        update_value(is_starting_script_at_start_up, value);
    };
    uint32_t get_message_buffer_size() override{
        return message_buffer_size;
    }
    void set_message_buffer_size(uint32_t value) override{
        update_value(message_buffer_size, value);
    }
    bool get_developer_log() override{
        return developer_log;
    }
    void set_developer_log(bool value) override{
        update_value(developer_log, value);
    }
    const char* get_pre_run_script() override{
        return pre_run_script.c_str();
    }
    void set_pre_run_script(const char* value) override{
        update_value(pre_run_script, value);
    }
    bool get_pre_run_script_is_valid() override{
        return pre_run_script_is_valid;
    }
    void set_pre_run_script_is_valid(bool value) override{
        update_value(pre_run_script_is_valid, value);
    }
    MAPPER_OPTION_RENDERING_METHOD get_rendering_method() override{
        return rendering_method;
    }
    void set_rendering_method(MAPPER_OPTION_RENDERING_METHOD value) override{
        update_value(rendering_method, value);
    }
    bool get_use_separated_ui_thread() override{
        return use_separated_ui_thread;
    }
    void set_use_separated_ui_thread(bool value) override{
        update_value(use_separated_ui_thread, value);
    }
    bool get_plugin_folder_is_default() override{
        return plugin_folder_is_default;
    }
    void set_plugin_folder_is_default(bool value) override{
        update_value(plugin_folder_is_default, value);
    }
    const std::filesystem::path &get_default_plugin_folder() override{
        return fs.get_default_plugin_path();
    }
    const std::filesystem::path &get_custom_plugin_folder() override{
        return custom_plugin_folder;
    }
    void set_custom_plugin_folder(std::filesystem::path &&path) override{
        update_value(custom_plugin_folder, std::move(path));
    }
    uint64_t get_lua_standard_libraries() override{
        return lua_standard_libraries;
    }
    void set_lua_standard_libraries(uint64_t value) override{
        update_value(lua_standard_libraries, value);
    }
    const char* get_skipped_version() override{
        return skipped_version.c_str();
    }
    void set_skipped_version(const char* value) override{
        update_value(skipped_version, value);
    }
    dcs_exporter_mode get_dcs_exporter_mode() override{
        return dcs_exporter_mode_data;
    }
    void set_dcs_exporter_mode(dcs_exporter_mode value) override{
        update_value(dcs_exporter_mode_data, value);
    }
    bool get_touch_mouse_emulation_is_enable() override{
        return touch_mouse_emulation;
    }
    void set_touch_mouse_emulation_is_enable(bool value) override{
        update_value(touch_mouse_emulation, value);
    }
    uint32_t get_touch_down_delay() override{
        return touch_down_delay;
    }
    void set_touch_down_delay(uint32_t value) override{
        update_value(touch_down_delay, value);
    }
    uint32_t get_touch_up_delay() override{
        return touch_up_delay;
    }
    void set_touch_up_delay(uint32_t value) override{
        update_value(touch_up_delay, value);
    }
    uint32_t get_touch_drag_start_delay() override{
        return touch_drag_start_delay;
    }
    void set_touch_drag_start_delay(uint32_t value) override{
        update_value(touch_drag_start_delay, value);
    }
    bool get_touch_double_tap_on_drag() override{
        return touch_double_tap_on_drag;
    }
    void set_touch_double_tap_on_drag(bool value) override{
        update_value(touch_double_tap_on_drag, value);
    }
    uint32_t get_touch_deadzone_for_drag() override{
        return touch_deadzone_for_drag;
    }
    void set_touch_deadzone_for_drag(uint32_t value) override{
        update_value(touch_deadzone_for_drag, value);
    }
    uint32_t get_touch_pointer_jitter() override{
        return touch_pointer_jitter;
    }
    void set_touch_pointer_jitter(uint32_t value) override{
        update_value(touch_pointer_jitter, value);
    }
    uint32_t get_touch_move_trigger_distance() override{
        return touch_move_trigger_distance;
    }
    void set_touch_move_trigger_distance(uint32_t value) override{
        update_value(touch_move_trigger_distance, value);
    }
    void reset_touch_delay() override{
        update_value(touch_down_delay, default_touch_down_delay);
        update_value(touch_up_delay, default_touch_up_delay);
        update_value(touch_drag_start_delay, default_touch_drag_start_delay);
        update_value(touch_double_tap_on_drag, default_touch_double_tap_on_drag);
        update_value(touch_deadzone_for_drag, default_touch_deadzone_for_drag);
        update_value(touch_pointer_jitter, default_touch_pointer_jitter);
        update_value(touch_move_trigger_distance, default_touch_move_trigger_distance);
    }

    void load_cli_params() override{
        std::ifstream ifs(cli_params_path.string());
        json data;
        ifs >> data;
        reflect_bool(data, CONFIG_CLI_LAUNCH_MINIMIZED, cli_launch_minimized);
        std::string path;
        reflect_string(data, CONFIG_CLI_SCRIPT_PATH, path);
        if (path.size() > 0){
            cli_script_path = std::move(std::filesystem::path(path));
        }else{
            cli_script_path = std::nullopt;
        }
    }
    void save_cli_params() override{
        json data{
            {CONFIG_CLI_LAUNCH_MINIMIZED, cli_launch_minimized},
            {CONFIG_CLI_SCRIPT_PATH, cli_script_path ? cli_script_path->string() : ""},
        };
        std::ofstream os(cli_params_path.string());
        os << data;
        os.close();
    }

    bool get_cli_launch_minimized() override{
        return cli_launch_minimized;
    }
    void set_cli_launch_minimized(bool value) override{
        cli_launch_minimized = value;
    }
    const std::optional<std::filesystem::path>& get_cli_script_path() override{
        return cli_script_path;
    }
    void set_cli_script_path(const wchar_t* path) override{
        if (path){
            cli_script_path = std::move(std::filesystem::path(path));
        }else{
            cli_script_path = std::nullopt;
        }
    }
};

static config_imp the_config;

namespace fsmapper{
    void init_app_config(){
        the_config.load();
    }

    config& app_config = the_config;
}
