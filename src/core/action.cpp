//
// action.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include <sstream>
#include "engine.h"
#include "action.h"

//============================================================================================
// C++ native action
//============================================================================================
NativeAction::NativeAction(const sol::object& object) : Action(object){
    function = object.as<std::shared_ptr<Function>>();
}

const char* NativeAction::getName(){
    return function->getName();
}

void NativeAction::invoke(Event& event, sol::state& lua){
    function->invoke(event, lua);
}

//============================================================================================
// action that invoke Lua function
//============================================================================================
LuaAction::LuaAction(const sol::object& object): Action(object), function(object){
}

const char* LuaAction::getName(){
    return "Lua function";
}

void LuaAction::invoke(Event &event, sol::state& lua){
    sol::protected_function_result result;
    auto type = event.getType();
    if (event.isArrayValue()){
        auto table = lua.create_table();
        event.applyToTable(table);
        result = function(event.getId(), table);
    }else if (type == Event::Type::null){
        result = function(event.getId());
    }else if (type == Event::Type::bool_value){
        result = function(event.getId(), event.getAs<bool>());
    }else if (type == Event::Type::int_value){
        result = function(event.getId(), event.getAs<int64_t>());
    }else if (type == Event::Type::double_value){
        result = function(event.getId(), event.getAs<double>());
    }else if (type == Event::Type::string_value){
        result = function(event.getId(), event.getAs<const char*>());
    }else if (type == Event::Type::lua_value){
        result = function(event.getId(), event.getAs<sol::object>());
    }
    
    if (!result.valid()){
        sol::error err = result;
        throw MapperException(err.what());
    }
}

//============================================================================================
// interpret event-action mapping definition as lua table
//============================================================================================
std::unique_ptr<EventActionMap> createEventActionMap(const MapperEngine& engine, const sol::object& def_o){
    auto map = std::make_unique<EventActionMap>();
    addEventActionMap(engine, map, def_o);
    return std::move(map);
}

void addEventActionMap(const MapperEngine& engine, const std::unique_ptr<EventActionMap>& map, const sol::object &def_o){
    if (def_o.get_type() == sol::type::table){
        sol::table def = def_o;
        for (int i = 1; i <= def.size(); i++){
            auto item = def[i];
            if (item.get_type() == sol::type::table){
                sol::table event_action = item;
                sol::object event = event_action["event"];
                if (event.get_type() != sol::type::number){
                    throw MapperException("The value of \"event\" parameter in event-action mapping "
                                          "definition is invalid, or there is no \"event\" parameter.");
                }
                auto evid = event.as<uint64_t>();
                auto evname = engine.getEventName(evid);
                if (evname == nullptr){
                    std::ostringstream os;
                    os << "Invalid event id is specified as event-action mapping: [" << evid << "]";
                    throw MapperException(os.str());
                }
                
                sol::object action = event_action["action"];
                if (action.get_type() == sol::type::function){
                    map->emplace(evid, std::make_unique<LuaAction>(action));
                }else if (action.is<NativeAction::Function&>()){
                    map->emplace(evid, std::make_unique<NativeAction>(action));
                }else{
                    std::ostringstream os;
                    os << "The value of \"action\" parameter in event-action mapping definition for "
                          "event \"" << evname << "\" is invalid, or there is no \"action\" parameter.";
                    throw MapperException(os.str());
                }
            }
        }
    }
}