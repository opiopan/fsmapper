//
// dcs_installer.hpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once

#include <filesystem>
#include "config.hpp"

namespace dcs {

    enum class exporter_status{
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

        enum class mode{install, uninstall};

        installer();
        bool check();
        bool install(mode mode = mode::install);
        winrt::Windows::Foundation::IAsyncAction show_install_error();
    };

    constexpr auto confirmation_unknown = 0;
    constexpr auto confirmation_yes = 1; 
    constexpr auto confirmation_no = 2;
    constexpr auto no_changes_needed = 3;
    winrt::Windows::Foundation::IAsyncOperation<int> confirm_change_export_lua(fsmapper::config::dcs_exporter_mode next_mode, installer& status);
}