//
// vjoy.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once

#include <sol/sol.hpp>

class MapperEngine;

class vJoyManager{
protected:
    MapperEngine& engine;

public:
    vJoyManager(MapperEngine& engine);
    ~vJoyManager();
    vJoyManager() = delete;
    vJoyManager(const vJoyManager&) = delete;
    vJoyManager(vJoyManager&&) = delete;

    MapperEngine& getEngine() const {return engine;}

    void init_scripting_env(sol::state& lua, sol::table& mapper_table);
};
