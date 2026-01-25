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

    std::vector<wchar_t> buf;
    buf.resize(256);
    while(true){
        if (::GetModuleFileNameW(nullptr, &buf.at(0), static_cast<DWORD>(buf.size())) < buf.size()){
            break;
        }
        buf.resize(buf.size() + 256);
    }
    module_dir = &buf.at(0);
    default_plugin = module_dir;
    default_plugin.remove_filename();
    default_plugin = default_plugin / "plugins";
    default_plugin2 = appdata / "plugins";
}