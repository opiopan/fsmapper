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
    std::string plugin_folder;
    int64_t stdlib{0};
    bool is_dcs_exporter_enabled{false};

    bool set_value(MAPPER_OPTION type, const char* value);
    bool set_value(MAPPER_OPTION type, int64_t value);
    bool set_value(MAPPER_OPTION type, bool value);
};
