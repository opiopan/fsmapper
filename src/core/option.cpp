//
// option.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

# include "option.h"

#include <unordered_map>

static std::unordered_map<MAPPER_OPTION, std::string MapperOption::*> string_options{
    {MOPT_PRE_RUN_SCRIPT, &MapperOption::pre_run_script},
    {MOPT_PLUGIN_FOLDER, &MapperOption::plugin_folder},
};

static std::unordered_map<MAPPER_OPTION, int64_t MapperOption::*> integer_options{
    {MOPT_RENDERING_METHOD, &MapperOption::rendering_method},
    {MOPT_STDLIB, &MapperOption::stdlib},
};

bool MapperOption::set_value(MAPPER_OPTION type, const char* value){
    if (string_options.count(type) > 0){
        this->*string_options.at(type) = value;
        return true;
    }else{
        return false;
    }
}

bool MapperOption::set_value(MAPPER_OPTION type, int64_t value){
    if (integer_options.count(type) > 0){
        this->*integer_options.at(type) = value;
        return true;
    }else{
        return false;
    }
}
