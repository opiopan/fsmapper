//
// viewobject.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once

#include <optional>
#include <sol/sol.hpp>
#include "tools.h"
#include "action.h"

class MapperEngine;
class Event;
namespace graphics{
    class render_target;
}

class ViewObject{
public:
    enum class touch_event{down, up, drag, cancel};
    enum class touch_reaction{none, capture, uncapture};

    virtual std::optional<float> get_aspect_ratio() = 0;
    virtual float calculate_scale_factor(const FloatRect& actual_region, float base_scale_factor) = 0;
    virtual touch_reaction process_touch_event(touch_event event, float rel_x, float rel_y, const FloatRect& actual_region) = 0;
    virtual void reset_touch_status() = 0;
    virtual void set_value(std::unique_ptr<Event>& value) = 0;
    virtual void merge_dirty_rect(const FloatRect& actual_region, FloatRect& dirty_rect) = 0;
    virtual void update_rect(graphics::render_target& target, const FloatRect& actual_region, float scale_factor) = 0;

    void set_value_lua(sol::object value);
    std::shared_ptr<NativeAction::Function> value_setter();
};

class Renderer{
public:
    virtual void render(graphics::render_target& target, const FloatRect& target_rect, float scale_factor, Event& value, sol::state& lua) = 0;
};

template <typename FUNC>
class NativeRenderer : public Renderer{
    FUNC function;
public:
    NativeRenderer(FUNC& function): function(function){}
    virtual ~NativeRenderer() = default;
    void render(graphics::render_target& target, const FloatRect& target_rect, float scale_factor, Event& value, sol::state& lua) override{
        function(target, target_rect, scale_factor, value);
    }
};

template <typename FUNC>
inline std::shared_ptr<NativeRenderer<FUNC>> make_native_renderer(FUNC function){
    return std::make_shared<NativeRenderer<FUNC>>(function);
}

void viewobject_init_scripting_env(MapperEngine& engine, sol::table& mapper_table);