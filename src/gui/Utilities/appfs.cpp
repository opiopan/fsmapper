//
// appfs.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include "pch.h"
#include <shlobj_core.h>
#include "appfs.hpp"

using namespace utils;

appfs::appfs(const wchar_t* appname){
    wchar_t* appdata_path {nullptr};
    SHGetKnownFolderPath(FOLDERID_RoamingAppData, KF_FLAG_DEFAULT, nullptr, &appdata_path);
    appdata = appdata_path;
    appdata /= appname;
    std::filesystem::create_directories(appdata);

    wchar_t* localdata_path {nullptr};
    SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_DEFAULT, nullptr, &localdata_path);
    localdata = localdata_path;
    localdata /= appname;
    std::filesystem::create_directories(localdata);
}