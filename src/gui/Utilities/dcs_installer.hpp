//
// dcs_installer.hpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once

#include <filesystem>

namespace dcs {

    enum class exporter_status
    {
        unknown,
        no_dcs,
        no_file,
        is_not_regular_file,
        failed_to_read,
        no_exporter,
        multiple_exporter,
        other_exporter,
        installed,
    };

    struct installer{
        std::filesystem::path dcs_env_path;
        std::filesystem::path export_lua_path;
        std::filesystem::path exsisting_base_path;
        std::filesystem::path base_path;
        exporter_status status;

        installer();
        bool check();
        bool install();
    };

}