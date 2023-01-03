//
// appfs.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include "pch.h"
#include "mappercore.h"
#include "appfs.hpp"

using namespace utils;

appfs::appfs(const wchar_t* appname){
    wchar_t* appdata_path {nullptr};
    mapper_getAppDataPath(&appdata_path);
    appdata = appdata_path;
    appdata /= appname;
    std::filesystem::create_directories(appdata);

    wchar_t* localdata_path {nullptr};
    mapper_getLocalDataPath(&localdata_path);
    localdata = localdata_path;
    localdata /= appname;
    std::filesystem::create_directories(localdata);
}