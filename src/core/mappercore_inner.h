//
// mappercore_inner.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once

#include <string>

struct CapturedWindowInfo{
    uint32_t cwid;
    std::string name;
    std::string target_class;
    bool is_captured;
};

struct ViewportInfo{
    std::string name;
    std::vector<std::string> views;
    ViewportInfo(const char* name, std::vector<std::string>&& views) : name(name), views(std::move(views)){}
};

class MapperEngine;
MapperEngine* mapper_EngineInstance();