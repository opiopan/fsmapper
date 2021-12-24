//
// event.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once

#include <string>
#include <map>
#include <memory>
#include <cmath>
#include <sol/sol.hpp>

class EventValue{
public:
    enum class Type{
        null,
        bool_value,
        int_value,
        double_value,
        string_value,
        lua_value,
    };

protected:
    Type type;
    union{
        bool boolValue;
        int64_t intValue;
        double doubleValue;
    }unionValue;
    std::unique_ptr<std::string> stringValue;
    sol::object luaValue;

public:
    EventValue(): type(Type::null){};
    EventValue(bool value): type(Type::bool_value){
        unionValue.boolValue = value;
    };
    EventValue(int64_t value): type(Type::int_value){
        unionValue.intValue = value;
    };
    EventValue(double value): type(Type::double_value){
        unionValue.doubleValue = value;
    };
    EventValue(const char* value) : type(Type::string_value){
        stringValue = std::make_unique<std::string>(value);
    };
    EventValue(std::string&& value) : type(Type::string_value){
        stringValue = std::make_unique<std::string>(std::move(value));
    };
    EventValue(const sol::object& value){
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
            stringValue = std::make_unique<std::string>(value.as<const char*>());
        }else{
            type = Type::lua_value;
            luaValue = value;
        }
    }
    EventValue(const EventValue& src){
        *this = src;
    };
    EventValue(EventValue&& src){
        *this = std::move(src);
    };
    ~EventValue() = default;

    EventValue& operator = (const EventValue& src){
        type = src.type;
        unionValue = src.unionValue;
        if (src.stringValue.get()){
            stringValue = std::make_unique<std::string>(*src.stringValue);
        }
        luaValue = src.luaValue;
        return *this;
    }
    EventValue& operator = (EventValue&& src){
        type = src.type;
        unionValue = src.unionValue;
        stringValue = std::move(src.stringValue);
        luaValue = src.luaValue;
        return *this;
    }

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
    operator std::string () const{
        return std::move(std::string(this->operator const char*()));
    };
    operator sol::object () const{
        return luaValue;
    };

    template <class T> T getAs() const {return static_cast<T>(*this);};
};


class Event{
public:
    using Type = EventValue::Type;
    using AssosiativeArray = std::map <std::string, EventValue>;

protected:
    uint64_t id;
    EventValue value;
	std::unique_ptr<AssosiativeArray> array;

public:
    Event() = delete;
    Event(uint64_t id): id(id), value(){};
    Event(uint64_t id, bool value): id(id), value(value){};
    Event(uint64_t id, int64_t value): id(id), value(value){};
    Event(uint64_t id, double value): id(id), value(value){};
    Event(uint64_t id, const char* value) : id(id), value(value){};
    Event(uint64_t id, std::string&& value) : id(id), value(std::move(value)){};
    Event(uint64_t id, sol::object&& value): id(id), value(std::move(value)){};
    Event(uint64_t id, AssosiativeArray&& value): id(id), array(std::make_unique<AssosiativeArray>(std::move(value))){};
    Event(const Event& src){*this = src;};
    Event(Event&& src): id(src.id), value(std::move(src.value)), array(std::move(src.array)){};
    ~Event() = default;

    Event& operator = (const Event& src){
        id = src.id;
        value = std::move(src.value);
        if (src.array.get()){
            array = std::make_unique<AssosiativeArray>(*src.array);
        }
        return *this;
    }
    Event& operator = (Event&& src){
        id = src.id;
        value = std::move(src.value);
        array = std::move(src.array);
        return *this;
    }

    uint64_t getId() const{return id;};
    bool isArrayValue() const{return array.get();};
    Type getType() const{return value.getType();};

    operator bool () const{
        return static_cast<bool>(value);
    };
    operator int64_t () const{
        return static_cast<int64_t>(value);
    };
    operator double () const{
        return static_cast<double>(value);
    };
    operator const char* () const{
        return static_cast<const char*>(value);
    };
    operator std::string () const{
        return std::move(std::string(this->operator const char*()));
    };
    operator sol::object () const{
        return static_cast<sol::object>(value);
    };
    operator const AssosiativeArray& () const{
        return *array;
    };

    template <class T> T getAs() const {return static_cast<T>(*this);};

    void applyToTable(sol::table& table){
        for (const auto& [key, value] : *array){
            switch (value.getType()){
            case Type::null:
                table[key] = nullptr;
                break;
            case Type::bool_value:
                table[key] = value.getAs<bool>();
                break;
            case Type::int_value:
                table[key] = value.getAs<int64_t>();
                break;
            case Type::double_value:
                table[key] = value.getAs<int64_t>();
                break;
            case Type::string_value:
                table[key] = value.getAs<const char*>();
                break;
            case Type::lua_value:
                table[key] = value.getAs<sol::object>();
                break;
            }
        }
    }
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
