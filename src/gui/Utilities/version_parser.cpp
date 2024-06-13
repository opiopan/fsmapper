//
// version_parser.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include "pch.h"
#include "version_parser.hpp"
#include "../.version.h"
#undef max
#include <algorithm>

namespace utils{
    parsed_version::parsed_version(const wchar_t* version){
        int curnum{0};
        for (; *version; version++){
            if (*version >= L'0' && *version <= L'9'){
                curnum *= 10;
                curnum += *version - L'0';
            }else{
                units.push_back(curnum);
                curnum = *version == L'.' ? 0 : 100;
            }
        }
        units.push_back(curnum);
    }

    int parsed_version::compare(const parsed_version &rval) const{
        auto to = std::max(units.size(), rval.units.size());
        for (auto i =0; i < to; i++){
            if (unit_at(i) > rval.unit_at(i)){
                return 1;
            }else if (unit_at(i) < rval.unit_at(i)){
                return -1;
            }
        }
        return 0;
    }

    const parsed_version this_version{L"" VERSTR_FILE_VERSION};
}