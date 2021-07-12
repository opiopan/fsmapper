//
// event.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once

#include <string>
#include <memory>
#include <cmath>
#include <sol/sol.hpp>

class Event{
public:
    enum class Type{
        null,
        bool_value,
        int_value,
        double_value,
        string_value,
        lua_value,
    } type;

protected:
    uint64_t id;
    union{
        bool boolValue;
        int64_t intValue;
        double doubleValue;
    }unionValue;
    std::unique_ptr<std::string> stringValue;
    sol::object luaValue;

public:
    Event() = delete;
    Event(uint64_t id): id(id), type(Type::null){};
    Event(uint64_t id, bool value): id(id), type(Type::bool_value){
        unionValue.boolValue = value;
    };
    Event(uint64_t id, int64_t value): id(id), type(Type::int_value){
        unionValue.intValue = value;
    };
    Event(uint64_t id, double value): id(id), type(Type::double_value){
        unionValue.doubleValue = value;
    };
    Event(uint64_t id, const char* value) : id(id), type(Type::string_value){
        stringValue.reset(new std::string(value));
    };
    Event(uint64_t id, std::string&& value) : id(id), type(Type::string_value){
        stringValue.reset(new std::string(std::move(value)));
    };
    Event(uint64_t id, sol::object&& value): id(id){
        auto valtype = value.get_type();
        if (valtype == sol::type::lua_nil){
            type = Type::null;
        }else if (valtype == sol::type::boolean){
            type = Type::null;
            unionValue.boolValue = value.as<bool>();
        }else if (valtype == sol::type::number){
            unionValue.doubleValue = value.as<double>();
            double intpart ;
            if (unionValue.doubleValue <= INT64_MAX && unionValue.doubleValue >= INT64_MIN &&
                std::modf(unionValue.doubleValue, &intpart) == 0.){
                type = Type::int_value;
                unionValue.intValue = unionValue.doubleValue;
            }else{
                type = Type::double_value;
            }
        }else if (valtype == sol::type::string){
            type = Type::string_value;
            stringValue.reset(new std::string(value.as<const char*>()));
        }else{
            type = Type::lua_value;
            luaValue = value;
        }
    }
    Event(Event&) = delete;
    Event(Event&&) = default;
    ~Event() = default;

    uint64_t getId() const{return id;};
    Type getType() const{return type;};

    operator bool () const{
        switch (type){
        case Type::null:
            return false;
        case Type::bool_value:
            return unionValue.boolValue;
        case Type::int_value:
            return unionValue.intValue;
        case Type::double_value:
            return unionValue.doubleValue;
        case Type::string_value:
            return stringValue->length();
        case Type::lua_value:
            return luaValue.get_type() != sol::type::lua_nil;
        }
    };
    operator int64_t () const{
        switch (type){
        case Type::bool_value:
            return unionValue.boolValue;
        case Type::int_value:
            return unionValue.intValue;
        case Type::double_value:
            return unionValue.doubleValue;
        case Type::null:
        case Type::string_value:
        case Type::lua_value:
            return 0;
        }
    };
    operator double () const{
        switch (type){
        case Type::bool_value:
            return unionValue.boolValue;
        case Type::int_value:
            return unionValue.intValue;
        case Type::double_value:
            return unionValue.doubleValue;
        case Type::null:
        case Type::string_value:
        case Type::lua_value:
            return 0;
        }
    };
    operator const char* () const{
        switch (type){
        case Type::string_value:
            return stringValue->c_str();
        case Type::null:
        case Type::bool_value:
        case Type::int_value:
        case Type::double_value:
        case Type::lua_value:
            return "";
        }
    };
    operator std::string&& () const{
        return std::move(std::string(this->operator const char*()));
    };
    operator sol::object () const{
        return luaValue;
    };

    template <class T> T getAs(){return static_cast<T>(*this);};
};

enum class EventID : int64_t{
    STOP = 1000,
    CHANGE_SIMCONNECTION,
    CHANGE_AIRCRAFT,
    CHANGE_DEVICES,
    READY_TO_CAPTURE_WINDOW,
    LOST_CAPUTRED_WINDOW,
    ENABLE_VIEWPORT,
    DISABLE_VIEPORT,
    
    DINAMIC_EVENT = 10000
};
