//
// filter.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include <memory>
#include <vector>
#include <sstream>
#include "filter.h"
#include "action.h"
#include "engine.h"

//============================================================================================
// Event duplicator
//============================================================================================
static std::shared_ptr<NativeAction::Function> duplicator(sol::variadic_args& va){
    std::ostringstream os;
    os << "filter.duplicate(";
    const char* prefix = "";
    std::vector<std::shared_ptr<NativeAction::Function>> actions;
    for (sol::object arg : va){
        if (!arg.is<NativeAction::Function&>()){
            throw std::runtime_error("only native action can be specified");
        }
        auto action = arg.as<std::shared_ptr<NativeAction::Function>>();
        actions.push_back(action);
        os << prefix << action->getName();
        prefix = ", ";
    }
    os << ")";
    
    NativeAction::Function::ACTION_FUNCTION func = [actions = std::move(actions)](Event& event){
        for (auto action : actions){
            action->invoke(event);
        }
    };
    return std::make_shared<NativeAction::Function>(os.str().c_str(), func);
};

//============================================================================================
// Create lua scripting environment
//============================================================================================
void filter_create_lua_env(MapperEngine& engine, sol::state& lua){
    auto table = lua.create_named_table("filter");

    table["duplicator"] = [&engine](sol::variadic_args va){
        return lua_c_interface(engine, "filter.duplicator", [&va]{
            return duplicator(va);
        });
    };
}