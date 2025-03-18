//
// cli_params.hpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include <string>

namespace fsmapper{
    struct cli_params {
        bool launch_minimized;
        const wchar_t* script_path;

        cli_params();

    protected:
        std::wstring script_path_buf;
    };
}