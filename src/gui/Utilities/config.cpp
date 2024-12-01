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
static const auto* CONFIG_PRE_RUN_SCRIPT = "pre_run_script";
static const auto* CONFIG_PRE_RUN_SCRIPT_IS_VALID = "pre_run_script_is_valid";
static const auto* CONFIG_RENDERING_METHOD = "rendering_method";
static const auto* CONFIG_PLUGIN_FOLDER_IS_DEFAULT = "plugin_folder_is_default";
static const auto* CONFIG_CUSTOM_PLUGIN_FOLDER = "custom_plugin_folder";
static const auto* CONFIG_LUA_STANDARD_LIBRARIES = "lua_stdlib";
static const auto* CONFIG_SKIPPED_VERSION = "skipped_version";
static const auto* CONFIG_DCS_EXPORTER_MODE = "dcs_exporter_mode";
static const auto* CONFIG_TOUCH_DOWN_DELAY = "touch_down_delay";
static const auto* CONFIG_TOUCH_UP_DELAY = "touch_up_delay";
static const auto *CONFIG_TOUCH_DRAG_START_DELAY = "touch_drag_start_delay";

static constexpr uint32_t default_touch_down_delay = 50;
static constexpr uint32_t default_touch_up_delay = 50;
static constexpr uint32_t default_touch_drag_start_delay = 150;

using namespace fsmapper;
using namespace nlohmann;

class config_imp : public config{
    bool is_dirty = false;

    utils::appfs fs{L"fsmapper"};
    std::filesystem::path config_path;

    rect window_rect = {0, 0, -1, -1};
    std::filesystem::path script_path;
    bool is_starting_script_at_start_up{true};
    uint32_t message_buffer_size{300};
    std::string pre_run_script;
    bool pre_run_script_is_valid{true};
    MAPPER_OPTION_RENDERING_METHOD rendering_method{MOPT_RENDERING_METHOD_CPU};
    bool plugin_folder_is_default{true};
    std::filesystem::path custom_plugin_folder;
    uint64_t lua_standard_libraries{MOPT_STDLIB_BASE | MOPT_STDLIB_PACKAGE | MOPT_STDLIB_MATH | MOPT_STDLIB_TABLE | MOPT_STDLIB_STRING};
    std::string skipped_version{"0.0"};
    dcs_exporter_mode dcs_exporter_mode_data{dcs_exporter_mode::unknown};
    uint32_t touch_down_delay{default_touch_down_delay};
    uint32_t touch_up_delay{default_touch_up_delay};
    uint32_t touch_drag_start_delay{default_touch_drag_start_delay};

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
        reflect_string(data, CONFIG_PRE_RUN_SCRIPT, pre_run_script);
        reflect_bool(data, CONFIG_PRE_RUN_SCRIPT_IS_VALID, pre_run_script_is_valid);
        reflect_number(data, CONFIG_RENDERING_METHOD, rendering_method);
        reflect_bool(data, CONFIG_PLUGIN_FOLDER_IS_DEFAULT, plugin_folder_is_default);
        path.clear();
        reflect_string(data, CONFIG_CUSTOM_PLUGIN_FOLDER, path);
        custom_plugin_folder = path;
        reflect_number(data, CONFIG_LUA_STANDARD_LIBRARIES, lua_standard_libraries);
        reflect_string(data, CONFIG_SKIPPED_VERSION, skipped_version);
        int dcs_exporter_mode_num{static_cast<int>(dcs_exporter_mode::unknown)};
        reflect_number(data, CONFIG_DCS_EXPORTER_MODE, dcs_exporter_mode_num);
        dcs_exporter_mode_data = static_cast<dcs_exporter_mode>(dcs_exporter_mode_num);
        reflect_number(data, CONFIG_TOUCH_DOWN_DELAY, touch_down_delay);
        reflect_number(data, CONFIG_TOUCH_UP_DELAY, touch_up_delay);
        reflect_number(data, CONFIG_TOUCH_DRAG_START_DELAY, touch_drag_start_delay);
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
                {CONFIG_PRE_RUN_SCRIPT, pre_run_script},
                {CONFIG_PRE_RUN_SCRIPT_IS_VALID, pre_run_script_is_valid},
                {CONFIG_RENDERING_METHOD, rendering_method},
                {CONFIG_PLUGIN_FOLDER_IS_DEFAULT, plugin_folder_is_default},
                {CONFIG_CUSTOM_PLUGIN_FOLDER, custom_plugin_folder.string()},
                {CONFIG_LUA_STANDARD_LIBRARIES, lua_standard_libraries},
                {CONFIG_SKIPPED_VERSION, skipped_version},
                {CONFIG_DCS_EXPORTER_MODE, static_cast<int>(dcs_exporter_mode_data)},
                {CONFIG_TOUCH_DOWN_DELAY, touch_down_delay},
                {CONFIG_TOUCH_UP_DELAY, touch_up_delay},
                {CONFIG_TOUCH_DRAG_START_DELAY, touch_drag_start_delay},
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
    bool get_is_starting_script_at_start_up(){
        return is_starting_script_at_start_up;
    };
    void set_is_starting_script_at_start_up(bool value){
        update_value(is_starting_script_at_start_up, value);
    };
    uint32_t get_message_buffer_size(){
        return message_buffer_size;
    }
    void set_message_buffer_size(uint32_t value){
        update_value(message_buffer_size, value);
    }
    const char* get_pre_run_script(){
        return pre_run_script.c_str();
    }
    void set_pre_run_script(const char* value){
        update_value(pre_run_script, value);
    }
    bool get_pre_run_script_is_valid(){
        return pre_run_script_is_valid;
    }
    void set_pre_run_script_is_valid(bool value){
        update_value(pre_run_script_is_valid, value);
    }
    MAPPER_OPTION_RENDERING_METHOD get_rendering_method(){
        return rendering_method;
    }
    void set_rendering_method(MAPPER_OPTION_RENDERING_METHOD value){
        update_value(rendering_method, value);
    }
    bool get_plugin_folder_is_default(){
        return plugin_folder_is_default;
    }
    void set_plugin_folder_is_default(bool value){
        update_value(plugin_folder_is_default, value);
    }
    const std::filesystem::path &get_default_plugin_folder(){
        return fs.get_default_plugin_path();
    }
    const std::filesystem::path &get_custom_plugin_folder()
    {
        return custom_plugin_folder;
    }
    void set_custom_plugin_folder(std::filesystem::path &&path){
        update_value(custom_plugin_folder, std::move(path));
    }
    uint64_t get_lua_standard_libraries(){
        return lua_standard_libraries;
    }
    void set_lua_standard_libraries(uint64_t value){
        update_value(lua_standard_libraries, value);
    }
    const char* get_skipped_version(){
        return skipped_version.c_str();
    }
    void set_skipped_version(const char* value){
        update_value(skipped_version, value);
    }
    dcs_exporter_mode get_dcs_exporter_mode(){
        return dcs_exporter_mode_data;
    }
    void set_dcs_exporter_mode(dcs_exporter_mode value){
        update_value(dcs_exporter_mode_data, value);
    }
    uint32_t get_touch_down_delay(){
        return touch_down_delay;
    }
    void set_touch_down_delay(uint32_t value){
        update_value(touch_down_delay, value);
    }
    uint32_t get_touch_up_delay(){
        return touch_up_delay;
    }
    void set_touch_up_delay(uint32_t value){
        update_value(touch_up_delay, value);
    }
    uint32_t get_touch_drag_start_delay(){
        return touch_drag_start_delay;
    }
    void set_touch_drag_start_delay(uint32_t value){
        update_value(touch_drag_start_delay, value);
    }
    void reset_touch_delay(){
        update_value(touch_down_delay, default_touch_down_delay);
        update_value(touch_up_delay, default_touch_up_delay);
        update_value(touch_drag_start_delay, default_touch_drag_start_delay);
    }
};

static config_imp the_config;

namespace fsmapper{
    void init_app_config(){
        the_config.load();
    }

    config& app_config = the_config;
}
