//
// viewobject.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once

#include <optional>
#include <sol/sol.hpp>
#include "tools.h"

class MapperEngine;
class EventValue;
namespace graphics{
    class render_target;
}

class ViewObject{
public:
    enum class touch_event{lbutton_down, lbutton_up, mouse_drag};
    enum class touch_reaction{none, capture, uncapture};

    virtual std::optional<float> get_aspect_ratio() = 0;
    virtual float claculate_scale_factor(const FloatRect& actual_region) = 0;
    virtual touch_reaction process_touch_event(touch_event event, float rel_x, float rel_y, const FloatRect& actual_region) = 0;
    virtual void reset_touch_status() = 0;
    virtual void set_value(const EventValue& value) = 0;
    virtual void merge_dirty_rect(const FloatRect& actual_region, FloatRect& dirty_rect) = 0;
    virtual void update_rect(graphics::render_target& target, const FloatRect& actual_region, float scale_factor) = 0;
};

void viewobject_init_scripting_env(MapperEngine& engine, sol::table& mapper_table);