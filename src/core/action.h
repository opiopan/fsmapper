//
// action.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once

#include <functional>
#include <optional>
#include <map>
#include <string>
#include <memory>
#include <sol/sol.hpp>
#include "event.h"

class MapperEngine;

class Action{
protected:
    sol::object lua_object;

public:
    Action(const Action&) = delete;
    Action(Action&&) = delete;
    Action(const sol::object& object): lua_object(object){};
    virtual ~Action() = default;

    virtual const char* getName() = 0;
    virtual void invoke(Event& event, sol::state& lua) = 0;
};

class NativeAction: public Action{
public:
    class Function {
    public:
        using ACTION_FUNCTION = std::function<void (Event&)>;
    protected:
        std::string name;
        ACTION_FUNCTION action;
    public:
        Function() = delete;
        Function(const Function&) = delete;
        Function(Function&&) =delete;
        Function(const char* name, ACTION_FUNCTION& action): name(name), action(action){};
        ~Function() = default;
        const char* getName(){return name.c_str();};
        void invoke(Event& event){action(event);};
    };

protected:
    Function* function;

public:
    NativeAction() = delete;
    NativeAction(const NativeAction&) = delete;
    NativeAction(NativeAction&&) = delete;
    NativeAction(const sol::object& object);
    virtual ~NativeAction() = default;
    virtual const char* getName();
    virtual void invoke(Event& event, sol::state& lua);
};

class LuaAction: public Action{
protected:
    sol::protected_function function;

public:
    LuaAction() = delete;
    LuaAction(const NativeAction&) = delete;
    LuaAction(NativeAction&&) = delete;
    LuaAction(const sol::object& object);
    virtual ~LuaAction() = default;
    virtual const char* getName();
    virtual void invoke(Event& event, sol::state& lua);
};

using EventActionMap = std::map<uint64_t, std::unique_ptr<Action>>;
std::unique_ptr<EventActionMap> createEventActionMap(const MapperEngine& engine, const sol::object &def);
