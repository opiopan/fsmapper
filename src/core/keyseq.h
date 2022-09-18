//
// keyseq.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once

#include <sol/sol.hpp>

class MapperEngine;

namespace keyseq{
    void create_lua_env(MapperEngine& engine, sol::table& mapper_table);
}