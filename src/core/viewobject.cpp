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
    static constexpr auto op_horizontal_flic = 4;
    static constexpr auto op_rotate = 8;
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
    FloatPoint initial_point;

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
        event_flic_up = lua_safevalue<int64_t>(def["event_flick_up"]);
        event_flic_down = lua_safevalue<int64_t>(def["event_flick_down"]);
        event_flic_right = lua_safevalue<int64_t>(def["event_flick_right"]);
        event_flic_left = lua_safevalue<int64_t>(def["event_flick_left"]);
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

    float calculate_scale_factor(const FloatRect& actual_region, float base_scale_factor) override{
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
                initial_point.x = rel_x;
                initial_point.y = rel_y;
                invaridate();
                return touch_reaction::capture;
            }else{
                return touch_reaction::none;
            }
        }else if (status == op_status::touch_in){
            if (event == touch_event::up){
                status = op_status::init;
                invaridate();
                if (valid_ops & op_horizontal_flic){
                    auto hdelta = rel_x - initial_point.x;
                    if (hdelta > 0.4 || (initial_point.x > 0.6 && rel_x > 0.6)){
                        mapper_EngineInstance()->sendEvent(std::move(Event(*event_flic_right)));
                    }else if (hdelta < -0.4 || (initial_point.x < 0.4 && rel_x < 0.4)){
                        mapper_EngineInstance()->sendEvent(std::move(Event(*event_flic_left)));
                    }
                }else if (valid_ops & op_vertical_flic){
                    auto vdelta = rel_y - initial_point.y;
                    if (vdelta > 0.4 || (initial_point.y > 0.6 && rel_y > 0.6)){
                        mapper_EngineInstance()->sendEvent(std::move(Event(*event_flic_down)));
                    }else if (vdelta < -0.4 || (initial_point.y < 0.4 && rel_y < 0.4)){
                        mapper_EngineInstance()->sendEvent(std::move(Event(*event_flic_up)));
                    }
                }else if (valid_ops & op_tap){
                    mapper_EngineInstance()->sendEvent(std::move(Event(*event_tap)));
                }
                return touch_reaction::uncapture;
            }else if (event == touch_event::cancel){
                status = op_status::init;
                invaridate();
                return touch_reaction::uncapture;
            }else if (event == touch_event::drag && out_of_region && !(valid_ops & ~op_tap)){
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
// LuaRenderer: expression of rederer which written as Lua script
//============================================================================================
class lua_renderer : public Renderer{
    sol::protected_function function;
public:
    lua_renderer() = delete;
    lua_renderer(sol::function function): function(function){}
    virtual ~lua_renderer() = default;
    void render(graphics::render_target& target, const FloatRect& target_rect, float scale_factor, Event& value, sol::state& lua) override;
};

void lua_renderer::render(graphics::render_target& target, const FloatRect& target_rect, float scale_factor, Event& value, sol::state& lua){
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
// canvas: expression of rederer which written as Lua script
//============================================================================================
class canvas : public ViewObject{
    std::optional<view_utils::region_restriction> def_restriction;
    std::unique_ptr<Event> value = std::make_unique<Event>(static_cast<int64_t>(EventID::NILL));
    bool is_dirty = true;
    std::shared_ptr<Renderer> renderer;

public:
    canvas() = delete;

    canvas(sol::object& def_obj){
        if (def_obj.get_type() != sol::type::table){
            throw MapperException("canvas definition must be specified as a table");
        }
        auto def = def_obj.as<sol::table>();

        def_restriction = view_utils::parse_region_restriction(def);
        sol::object value = def["value"];
        if (value.get_type() != sol::type::lua_nil){
            this->value = std::make_unique<Event>(static_cast<int64_t>(EventID::NILL), std::move(value));
        }
        sol::object renderer = def["renderer"];
        if (renderer.is<Renderer&>()){
            this->renderer = renderer.as<std::shared_ptr<Renderer>>();
        }else if (renderer.get_type() == sol::type::function){
            this->renderer = std::make_shared<lua_renderer>(renderer);
        }else{
            throw MapperException("no renderer parameter is specified or invarid object is specifid for renderer parameter");
        }
    }

    virtual ~canvas() = default;

    //-----------------------------------------------------------------------------------
    // ViewObject interface implementation
    //-----------------------------------------------------------------------------------
    std::optional<float> get_aspect_ratio() override{
        return def_restriction ? std::optional<float>(def_restriction->aspect_ratio) : std::nullopt;
    }

    float calculate_scale_factor(const FloatRect& actual_region, float base_scale_factor) override{
        return def_restriction && def_restriction->logical_size ? 
            actual_region.width / def_restriction->logical_size->width : base_scale_factor;
    }

    touch_reaction process_touch_event(touch_event event, float rel_x, float rel_y, const FloatRect& actual_region) override{
        return touch_reaction::none;
    }

    void reset_touch_status() override{}

    void set_value(std::unique_ptr<Event>& value) override{
        bool need_update = true;
        auto ctype = this->value->getType();
        auto ntype = value->getType();
        if (ntype == ctype){
            if (ntype == Event::Type::null){
                need_update = false;
            }else if (ntype == Event::Type::bool_value){
                need_update = value->getAs<bool>() != this->value->getAs<bool>();
            }else if (ntype == Event::Type::int_value){
                need_update = value->getAs<int64_t>() != this->value->getAs<int64_t>();
            }else if (ntype == Event::Type::double_value){
                need_update = value->getAs<double>() != this->value->getAs<double>();
            }
        }
        if (need_update){
            this->value = std::move(value);
            if (!is_dirty){
                is_dirty = true;
                mapper_EngineInstance()->invokeViewportsUpdate();
            }
        }
    }

    void merge_dirty_rect(const FloatRect& actual_region, FloatRect& dirty_rect) override{
        if (is_dirty){
            dirty_rect += actual_region;
        }
    }

    void update_rect(graphics::render_target& target, const FloatRect& actual_region, float scale_factor) override{
        renderer->render(target, actual_region, scale_factor, *value, mapper_EngineInstance()->getLuaState());
        is_dirty = false;
    }

    void set_value_lua(sol::object value){
        auto event = std::make_unique<Event>(static_cast<int64_t>(EventID::NILL), std::move(value));
        set_value(event);
    }

    std::shared_ptr<NativeAction::Function> value_setter(){
        NativeAction::Function::ACTION_FUNCTION func = [this](Event& event, sol::state&){
            auto value = std::make_unique<Event>(std::move(event));
            set_value(value);
        };
        return std::make_shared<NativeAction::Function>("canvas:set_value()", func);
    }

    sol::object get_value_lua(sol::this_state s){
        sol::state_view lua{s};

        if (value->getType() == Event::Type::bool_value){
            return sol::lua_value(lua, value->getAs<bool>()).as<sol::object>();
        }else if (value->getType() == Event::Type::double_value){
            return sol::lua_value(lua, value->getAs<double>()).as<sol::object>();
        }else if (value->getType() == Event::Type::int_value){
            return sol::lua_value(lua, value->getAs<int64_t>()).as<sol::object>();
        }else if (value->getType() == Event::Type::string_value){
            return sol::lua_value(lua, value->getAs<const char*>()).as<sol::object>();
        }else if (value->getType() == Event::Type::lua_value){
            return value->getAs<sol::object>();
        }

        return sol::lua_nil;
    }
};

//============================================================================================
// utility function to realize polimophism for lua object
//============================================================================================
std::shared_ptr<ViewObject> as_view_object(const sol::object& obj){
    if (obj.is<operable_area&>()){
        return obj.as<std::shared_ptr<operable_area>>();
    }else if (obj.is<canvas&>()){
        return obj.as<std::shared_ptr<canvas>>();
    }else{
        return nullptr;
    }
}

//============================================================================================
// building lua environment
//============================================================================================
void viewobject_init_scripting_env(MapperEngine& engine, sol::table& mapper_table){
    auto& lua = engine.getLuaState();
    auto table = lua.create_table();

    table.new_usertype<operable_area>(
        "operable_area",
        sol::call_constructor, sol::factories([&engine](sol::object def){
            return lua_c_interface(engine, "mapper.view_elements.operable_area", [&def]{
                return std::make_shared<operable_area>(def);
            });
        })
    );

    table.new_usertype<canvas>(
        "canvas",
        sol::call_constructor, sol::factories([&engine](sol::object def){
            return lua_c_interface(engine, "mapper.view_elements.canvas", [&def]{
                return std::make_shared<canvas>(def);
            });
        }), 
        "value", sol::property(&canvas::get_value_lua, &canvas::set_value_lua),
        "set_value", &canvas::set_value_lua,
        "get_value", &canvas::get_value_lua,
        "value_setter", &canvas::value_setter
    );

    mapper_table["view_elements"] = table;
}