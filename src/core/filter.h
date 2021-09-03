//
// filter.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once

#include <sol/sol.hpp>

class MapperEngine;

void filter_create_lua_env(MapperEngine& engine, sol::state& lua);