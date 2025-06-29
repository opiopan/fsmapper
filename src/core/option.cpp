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

static std::unordered_map<MAPPER_OPTION, bool MapperOption::*> boolean_options{
    {MOPT_ASYNC_MESSAGE_PUMPING, &MapperOption::async_message_pumping},
    {MOPT_DCS_EXPORTER, &MapperOption::is_dcs_exporter_enabled},
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

bool MapperOption::set_value(MAPPER_OPTION type, bool value){
    if (boolean_options.count(type) > 0){
        this->*boolean_options.at(type) = value;
        return true;
    }else{
        return false;
    }
}
