//
// mappercore_inner.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once

#include <string>

struct CapturedWindowInfo{
    uint32_t cwid;
    std::string name;
    bool is_captured;
};
