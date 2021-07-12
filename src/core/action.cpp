//
// action.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//


#include "action.h"

//============================================================================================
// C++ native action
//============================================================================================
NativeAction::NativeAction(const sol::object& object) : Action(object){
    function = &object.as<Function&>();
}

const char* NativeAction::getName(){
    return function->getName();
}

void NativeAction::invoke(Event& event){
    function->invoke(event);
}

//============================================================================================
// action that invoke Lua function
//============================================================================================
LuaAction::LuaAction(const sol::object& object): Action(object), function(object){
}

const char* LuaAction::getName(){
    return "Lua function";
}

void LuaAction::invoke(Event &event){
    sol::protected_function_result result;
    auto type = event.getType();
    if (type == Event::Type::null){
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
