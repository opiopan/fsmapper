//
// option.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once

#include <string>
#include "mappercore.h"

struct MapperOption{
    std::string pre_run_script;
    int64_t rendering_method {MOPT_RENDERING_METHOD_CPU};

    bool set_value(MAPPER_OPTION type, const char* value);
    bool set_value(MAPPER_OPTION type, int64_t value);
};
