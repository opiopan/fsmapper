//
// tool.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include <sol/sol.hpp>

inline std::string lua_safestring(const sol::object& object){
    if (object.get_type() == sol::type::string){
        return object.as<std::string>();
    }else{
        return std::string();
    }
};