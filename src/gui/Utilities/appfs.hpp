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
        std::filesystem::path default_plugin;

    public:
        appfs() = delete;
        appfs(const wchar_t* appname);
        appfs(const appfs&) = delete;
        appfs(appfs&&) = delete;

        const std::filesystem::path& get_appdata_path(){return appdata;}
        const std::filesystem::path& get_localdata_path(){return localdata;}
        const std::filesystem::path& get_default_plugin_path(){return default_plugin;}
    };
}
