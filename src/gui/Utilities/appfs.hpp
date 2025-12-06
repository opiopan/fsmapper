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
        std::filesystem::path module_dir;

    public:
        appfs() = delete;
        appfs(const wchar_t* appname);
        appfs(const appfs&) = delete;
        appfs(appfs&&) = delete;

        const std::filesystem::path& get_appdata_path(){return appdata;}
        const std::filesystem::path& get_localdata_path(){return localdata;}
        const std::filesystem::path& get_default_plugin_path(){return default_plugin;}
        const std::filesystem::path& get_module_dir(){return module_dir;}
    };
}
