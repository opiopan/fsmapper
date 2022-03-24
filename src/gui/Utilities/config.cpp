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

using namespace fsmapper;
using namespace nlohmann;

class config_imp : public config{
    bool is_dirty = false;

    utils::appfs fs{L"fsmapper"};
    std::filesystem::path config_path;

    rect window_rect = {0, 0, -1, -1};
    std::filesystem::path script_path;
    bool is_starting_script_at_start_up{true};
    int32_t message_buffer_size{300};

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

public:
    config_imp(){
        config_path = std::move(fs.get_appdata_path() / L"config.json");
    };

    void load(){
        if (!std::filesystem::exists(config_path)){
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
    int32_t get_message_buffer_size(){
        return message_buffer_size;
    }
    void set_message_buffer_size(int32_t value){
        update_value(message_buffer_size, value);
    }
};

static config_imp the_config;

namespace fsmapper{
    void init_app_config(){
        the_config.load();
    }

    config& app_config = the_config;
}
