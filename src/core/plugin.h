//
// plugin.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once

#include <string>
#include <vector>
#define NOMINMAX
#include <windows.h>
#include "mapperplugin.h"

class PluginManager{
    struct PluginModule{
        std::string file_name;
        HMODULE module;
        MAPPER_PLUGIN_DEVICE_OPS* ops;
        PluginModule(const std::string& file_name, HMODULE module, MAPPER_PLUGIN_DEVICE_OPS* ops) :
            file_name(file_name), module(module), ops(ops){}
    };
    std::vector<PluginModule> modules;

public:
    PluginManager();
    ~PluginManager();

    size_t get_plugin_num(){
        return modules.size();
    }
    const char* get_name_at(size_t index){
        return modules.at(index).ops->name;
    }
    const std::string& get_file_name_at(size_t index){
        return modules.at(index).file_name;
    }
    MAPPER_PLUGIN_DEVICE_OPS* get_ops_at(size_t index){
        return modules.at(index).ops;
    }
};