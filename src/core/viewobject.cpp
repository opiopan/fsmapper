//
// viewobject.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include "viewobject.h"

#include <memory>
#include <optional>
#include <algorithm>
#include "engine.h"
#include "graphics.h"
#include "capturedwindow.h"

//============================================================================================
// operable object
//============================================================================================
class operable_area : public ViewObject{
    enum class op_status{init, touch_in, touch_out};
    op_status status = op_status::init;
    bool is_dirty = true;
    static constexpr auto op_tap = 1;
    static constexpr auto op_vertical_flic = 2;
    static constexpr auto op_horizontal_flic = 3;
    static constexpr auto op_rotate = 4;
    uint32_t valid_ops = 0;
    graphics::color reaction_color{0.f, 1.f, 1.f, 0.3f};
    float round_ratio = 0.f;
    std::optional<int64_t> event_tap;
    std::optional<int64_t> event_flic_up;
    std::optional<int64_t> event_flic_down;
    std::optional<int64_t> event_flic_right;
    std::optional<int64_t> event_flic_left;
    std::optional<int64_t> event_rotate_clockwise;
    std::optional<int64_t> event_rotate_counter_clockwise;

public:
    operable_area() = delete;

    operable_area(sol::object& def_obj){
        if (def_obj.get_type() != sol::type::table){
            throw MapperException("operable_area definition must be specified as a table");
        }
        auto def = def_obj.as<sol::table>();

        sol::object color = def["reaction_color"];
        if (color){
            reaction_color = graphics::color(color);
        }
        auto round = lua_safevalue<float>(def["round_ratio"]);
        if (round){
            if (*round > 0.5f){
                round_ratio = 0.5;
            }else if (*round < 0.f){
                round_ratio = 0.f;
            }else{
                round_ratio = *round;
            }
        }
        event_tap = lua_safevalue<int64_t>(def["event_tap"]);
        event_flic_up = lua_safevalue<int64_t>(def["event_flic_up"]);
        event_flic_down = lua_safevalue<int64_t>(def["event_flic_down"]);
        event_flic_right = lua_safevalue<int64_t>(def["event_flic_right"]);
        event_flic_left = lua_safevalue<int64_t>(def["event_flic_left"]);
        event_rotate_clockwise = lua_safevalue<int64_t>(def["event_rotate_clockwise"]);
        event_rotate_counter_clockwise = lua_safevalue<int64_t>(def["event_rotate_counter_clockwise"]);
        valid_ops |= event_tap ? op_tap : 0;
        valid_ops |= (event_flic_up || event_flic_up) ? op_vertical_flic : 0;
        valid_ops |= (event_flic_right || event_flic_left) ? op_horizontal_flic : 0;
        valid_ops |= (event_rotate_clockwise || event_rotate_counter_clockwise) ? op_rotate : 0;
        if (valid_ops == 0){
            throw MapperException("no event specified");
        }
    }

    virtual ~operable_area() = default;

    //-----------------------------------------------------------------------------------
    // ViewObject interface implementation
    //-----------------------------------------------------------------------------------
    std::optional<float> get_aspect_ratio() override{
        return std::nullopt;
    }

    float claculate_scale_factor(const FloatRect& actual_region) override{
        return 1.f;
    }

    touch_reaction process_touch_event(touch_event event, float rel_x, float rel_y, const FloatRect& actual_region) override{
        auto out_of_region = rel_x < 0.f || rel_x > 1.f || rel_y < 0.f || rel_y > 1.f;
        auto invaridate = [this]{
            is_dirty = true;
            mapper_EngineInstance()->invokeViewportsUpdate();
        };
        if (status == op_status::init){
            if (event == touch_event::down){
                status = op_status::touch_in;
                invaridate();
                return touch_reaction::capture;
            }else{
                return touch_reaction::none;
            }
        }else if (status == op_status::touch_in){
            if (event == touch_event::up){
                status = op_status::init;
                invaridate();
                if (event_tap){
                    mapper_EngineInstance()->sendEvent(std::move(Event(*event_tap)));
                }
                return touch_reaction::uncapture;
            }else if (event == touch_event::cancel){
                status = op_status::init;
                invaridate();
                return touch_reaction::uncapture;
            }else if (event == touch_event::drag && out_of_region){
                status = op_status::touch_out;
                invaridate();
            }
            return touch_reaction::capture;
        }else if (status == op_status::touch_out){
            if (event == touch_event::up || event == touch_event::cancel){
                status = op_status::init;
                return touch_reaction::uncapture;
            }else if (event == touch_event::drag && !out_of_region){
                status = op_status::touch_in;
                invaridate();
            }
            return touch_reaction::capture;
        }
        return touch_reaction::none;
    }

    void reset_touch_status() override{
        status = op_status::init;
    }

    void set_value(std::unique_ptr<Event>& value) override{
    }

    void merge_dirty_rect(const FloatRect& actual_region, FloatRect& dirty_rect) override{
        if (is_dirty){
            dirty_rect += actual_region;
        }
    }

    void update_rect(graphics::render_target& target, const FloatRect& actual_region, float scale_factor) override{
        if (status == op_status::touch_in){
            auto round_radius = std::max(actual_region.width, actual_region.height) * round_ratio;
            D2D1_ROUNDED_RECT rrect{actual_region, round_radius, round_radius};
            target->FillRoundedRectangle(rrect, reaction_color.brush_interface(target));
        }
        is_dirty = false;
    }
};

//============================================================================================
// LuaRenderer implementation
//============================================================================================
void LuaRenderer::render(graphics::render_target& target, const FloatRect& target_rect, float scale_factor, Event& value, sol::state& lua){
    graphics::rendering_context ctx(target, target_rect, scale_factor);
    sol::protected_function_result result;
    auto type = value.getType();
    if (value.isArrayValue()){
        auto table = lua.create_table();
        value.applyToTable(table);
        result = function(ctx, table);
    }else if (type == Event::Type::null){
        result = function(ctx);
    }else if (type == Event::Type::bool_value){
        result = function(ctx, value.getAs<bool>());
    }else if (type == Event::Type::int_value){
        result = function(ctx, value.getAs<int64_t>());
    }else if (type == Event::Type::double_value){
        result = function(ctx, value.getAs<double>());
    }else if (type == Event::Type::string_value){
        result = function(ctx, value.getAs<const char*>());
    }else if (type == Event::Type::lua_value){
        result = function(ctx, value.getAs<sol::object>());
    }
    
    if (!result.valid()){
        sol::error err = result;
        throw MapperException(err.what());
    }
}

//============================================================================================
// building lua environment
//============================================================================================
void viewobject_init_scripting_env(MapperEngine& engine, sol::table& mapper_table){
    auto& lua = engine.getLuaState();
    auto table = lua.create_table();

    //
    // operable_area
    //
    table.new_usertype<operable_area>(
        "operable_area",
        sol::call_constructor, sol::factories([&engine](sol::object def){
            return lua_c_interface(engine, "mapper.view_elements.operable_area", [&def]{
                std::shared_ptr<ViewObject> ptr = std::make_shared<operable_area>(def);
                return ptr;
            });
        })
    );

    mapper_table["view_elements"] = table;
}