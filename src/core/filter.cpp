//
// filter.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include <memory>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <algorithm>
#include "filter.h"
#include "action.h"
#include "engine.h"
#include "tools.h"

//============================================================================================
// Utility functions
//============================================================================================
static std::shared_ptr<Action> generate_action(const sol::object& o_function){
    if (o_function.is<NativeAction::Function&>()){
        return std::make_shared<NativeAction>(o_function);
    }else if (o_function.get_type() == sol::type::function){
        return std::make_shared<LuaAction>(o_function);
    }else{
        return nullptr;
    }
}

//============================================================================================
// Event duplicator
//============================================================================================
static std::shared_ptr<NativeAction::Function> duplicator(sol::variadic_args& va){
    std::ostringstream os;
    os << "filter.duplicate(";
    const char* prefix = "";
    std::vector<std::shared_ptr<Action>> actions;
    for (sol::object arg : va){
        auto action = std::move(generate_action(arg));
        if (!action.get()){
            throw std::runtime_error("only native action or Lua function can be specified");
        }
        actions.push_back(action);
        os << prefix << action->getName();
        prefix = ", ";
    }
    os << ")";

    NativeAction::Function::ACTION_FUNCTION func = [actions = std::move(actions)](Event& event, sol::state& lua){
        for (auto& action : actions){
            action->invoke(event, lua);
        }
    };
    return std::make_shared<NativeAction::Function>(os.str().c_str(), func);
};

//============================================================================================
// interpolation filters
//============================================================================================
template <typename T>
struct value_map{
    T in;
    T out;
};
using double_map = value_map<double>;
using int_map = value_map<int64_t>;
template <typename T> using value_map_set = std::vector<value_map<T>>;
using double_map_set = value_map_set<double>;
using int_map_set = value_map_set<int64_t>;

enum class val_map_type{invalid, double_map, int_map};

static val_map_type lua_numeric_type(sol::object& o){
    if (o.get_type() == sol::type::number){
        auto dval = o.as<double>();
        double intpart;
        if (dval <= INT64_MAX && dval >= INT64_MIN && std::modf(dval, &intpart) == 0.){
            return val_map_type::int_map;
        }else{
            return val_map_type::double_map;
        }
    }else{
        return val_map_type::invalid;
    }
}

static val_map_type verify_map_rule(sol::object& o_list){
    if (o_list.get_type() == sol::type::table){
        auto valtype = val_map_type::invalid;
        auto list = o_list.as<sol::table>();
        for (int i = 1; i <= list.size(); i++){
            sol::object o_valmap = list[i];
            if (o_valmap.get_type() != sol::type::table){
                return val_map_type::invalid;
            }
            auto valmap = o_valmap.as<sol::table>();
            if (valmap.size() != 2){
                return val_map_type::invalid;
            }
            sol::object in_val = valmap[1];
            sol::object out_val = valmap[1];
            auto in_type = lua_numeric_type(in_val);
            auto out_type = lua_numeric_type(out_val);
            if (in_type == val_map_type::invalid || out_type == val_map_type::invalid){
                return val_map_type::invalid;
            }else if (in_type == val_map_type::int_map && out_type == val_map_type::int_map){
                valtype = (valtype == val_map_type::double_map ? val_map_type::double_map : val_map_type::int_map);
            }else{
                valtype = val_map_type::double_map;
            }
        }
        return valtype;
    }else{
        return val_map_type::invalid;
    }
}

template <typename T>
value_map_set<T> generate_value_map(sol::object& o_def){
    value_map_set<T> maps;
    auto def = o_def.as<sol::table>();
    for (int i = 1; i <= def.size(); i++){
        sol::table map = def[i];
        T in = map[1];
        T out = map[2];
        maps.push_back({in, out});
        if (i >= 2){
            if (maps[i - 2].in <= maps[i - 1].in){
                "reference value for input value in the interpolation rule must be monotonic increased";
            }
        }
    }
    return std::move(maps);
}

template <typename T>
struct leap_parameter{
    T in_range;
    T out_range;
    T in_bias;
    T out_bias;
};

template <typename T>
T translate_value(const value_map_set<T>& maps, T target){
    auto make_param = [](const value_map<T>& a, const value_map<T>& b, leap_parameter<T>& param){
        param.in_range = b.in - a.in;
        param.out_range = b.out - a.out;
        param.in_bias = a.in;
        param.out_bias = a.out;
    };
    int i = 0;
    for (;i < maps.size(); i++){
        auto& map = maps[i];
        if (map.in >= target){
            break;
        }
    }
    leap_parameter<T> param;
    if (i == 0){
        make_param(maps[0], maps[1], param);
    }else if (i == maps.size()){
        make_param(maps[i - 2], maps[i - 1], param);
    }else{
        make_param(maps[i - 1], maps[i], param);
    }
    return (target - param.in_bias) * param.out_range / param.in_range + param.out_bias;
}

static std::shared_ptr<NativeAction::Function> lerp(sol::object& o_action, sol::object& o_list){
    auto action = generate_action(o_action);
    if (!action.get()){
        throw std::runtime_error("1st argument must be native action or Lua function");
    }
    auto type = verify_map_rule(o_list);
    if (type == val_map_type::invalid){
        throw std::runtime_error("the interpolation rule specified at 2nd argument is invarid format");
    }
    std::optional<double_map_set> double_maps;
    std::optional<int_map_set> int_maps;
    if (type == val_map_type::double_map){
        double_maps = std::move(generate_value_map<double>(o_list));
    }
    if (type == val_map_type::int_map){
        int_maps = std::move(generate_value_map<int64_t>(o_list));
    }
    std::ostringstream os;
    os << "filter.lerp(" << action->getName() << ")";

    NativeAction::Function::ACTION_FUNCTION func = 
    [action, double_maps = std::move(double_maps), int_maps = std::move(int_maps)](Event& event, sol::state& lua){
        auto event_type = event.getType();
        if (event_type == Event::Type::double_value || event_type == Event::Type::int_value){
            if (double_maps){
                Event new_event(event.getId(), translate_value(*double_maps, event.getAs<double>()));
                action->invoke(new_event, lua);
            }else{
                Event new_event(event.getId(), translate_value(*int_maps, event.getAs<int64_t>()));
                action->invoke(new_event, lua);
            }
        }else{
            action->invoke(event, lua);
        }
    };
    return std::make_shared<NativeAction::Function>(os.str().c_str(), func);
}

//============================================================================================
// Conditional branch
//============================================================================================
struct branch_condition {
    enum class optype {exceeded, falled};
    optype type;
    int64_t value;
    std::shared_ptr<Action> action;

    branch_condition() = delete;
    branch_condition(const sol::object& object){
        if (object.get_type() == sol::type::table){
            auto defs = object.as<sol::table>();
            auto&& condition = lua_safestring(defs["condition"]);
            static const std::unordered_map<std::string, optype> optype_dict ={
                {"exceeded", optype::exceeded},
                {"falled", optype::falled},
            };
            if (optype_dict.count(condition) == 0){
                throw std::runtime_error("the value of \"condition\" parameter must be either \"exceeded\" or \"falled\"");
            }
            type = optype_dict.at(condition);
            auto o_value = lua_safevalue<int64_t>(defs["value"]);
            if (!o_value.has_value()){
                throw std::runtime_error("\"value\" parameter is not specified or it's value is invalid");
            }
            value = *o_value;
            action = generate_action(defs["action"]);
            if (!action.get()){
                throw std::runtime_error("the value of \"action\" parameter must be either native action or Lua function");
            }
        }else{
            throw std::runtime_error("invalid type of argument, you must specify tables");
        }
    };
    branch_condition(const branch_condition&) = default;
    branch_condition& operator = (const branch_condition&) = default;
};

static std::shared_ptr<NativeAction::Function> branch(sol::variadic_args& va){
    std::ostringstream os;
    os << "filter.branch(";
    const char* prefix = "";
    std::vector<branch_condition> conds_exeeded;
    std::vector<branch_condition> conds_falled;
    for (sol::object arg : va){
        branch_condition cond(arg);
        if (cond.type == branch_condition::optype::exceeded){
            conds_exeeded.push_back(cond);
        }else{
            conds_falled.push_back(cond);
        }
        os << prefix << cond.action->getName();
        prefix = ", ";
    }
    os << ")";
    std::sort(std::begin(conds_exeeded), std::end(conds_exeeded), [](const branch_condition& a, const branch_condition& b){
        return a.value < b.value;
    });
    std::sort(std::begin(conds_falled), std::end(conds_falled), [](const branch_condition& a, const branch_condition& b){
        return a.value > b.value;
    });
    int64_t last = 0;
    int ix_exeeded = 0;
    for (; ix_exeeded < conds_exeeded.size() && conds_exeeded[ix_exeeded].value < last; ix_exeeded++);
    int ix_falled = 0;
    for (; ix_falled < conds_falled.size() && conds_falled[ix_falled].value > last; ix_falled++);

    NativeAction::Function::ACTION_FUNCTION func = [
        conds_exeeded = std::move(conds_exeeded), conds_falled = std::move(conds_falled),
        last, ix_exeeded, ix_falled
    ](Event& event, sol::state& lua) mutable {
        auto evtype = event.getType();
        if (evtype == Event::Type::int_value || evtype == Event::Type::double_value || evtype == Event::Type::bool_value){
            auto value = event.getAs<int64_t>();
            if (value > last){
                if (ix_exeeded < conds_exeeded.size() && conds_exeeded[ix_exeeded].value < value){
                    conds_exeeded[ix_exeeded].action->invoke(event, lua);
                    ix_exeeded++;
                }
                if (ix_falled > 0 && conds_falled[ix_falled - 1].value < value){
                    ix_falled--;
                }
            }else if (value < last){
                if (ix_falled < conds_falled.size() && conds_falled[ix_falled].value > value){
                    conds_falled[ix_falled].action->invoke(event, lua);
                    ix_falled++;
                }
                if (ix_exeeded > 0 && conds_exeeded[ix_exeeded - 1].value > value){
                    ix_exeeded--;
                }
            }
            last = value;
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

    table["lerp"] = [&engine](sol::object action, sol::object list){
        return lua_c_interface(engine, "filter.lerp", [&action, &list]{
            return lerp(action, list);
        });
    };

    table["branch"] = [&engine](sol::variadic_args va){
        return lua_c_interface(engine, "filter.branch", [&va]{
            return branch(va);
        });
    };
}