//
// appfs.hpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once

#include <filesystem>

namespace utils{

    class appfs{
        std::filesystem::path appdata;
        std::filesystem::path localdata;

    public:
        appfs() = delete;
        appfs(const wchar_t* appname);
        appfs(const appfs&) = delete;
        appfs(appfs&&) = delete;

        const std::filesystem::path& get_appdata_path(){return appdata;}
        const std::filesystem::path& get_localdata_path(){return localdata;}
    };
}
