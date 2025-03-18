//
// cli_params.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include "pch.h"
#include "cli_params.hpp"

class full_path {
    std::vector<wchar_t> buf;
public:
    full_path(const wchar_t* rpath) {
        buf.resize(256);
        while (true) {
            buf[0] = 0;
            auto length = GetFullPathNameW(rpath, static_cast<DWORD>(buf.size()), &buf.at(0), nullptr);
            if (length > static_cast<DWORD>(buf.size())) {
                buf.resize(length);
            }
            else {
                break;
            }
        }
    }

    operator const wchar_t* () const {
        return &buf.at(0);
    }
};

struct command_line{
    int argc{0};
    wchar_t** argv{nullptr};

    command_line(){
        auto argument = ::GetCommandLineW();
        argv = CommandLineToArgvW(argument, &argc);
    }
    ~command_line(){
        ::LocalFree(argv);
    }
};

namespace fsmapper{
    cli_params::cli_params(): launch_minimized(false), script_path(nullptr){
        command_line args;
        int ix{1};
        for (; ix < args.argc; ix++){
            if (args.argv[ix][0] != L'/'){
                break;
            }
            if (wcscmp(args.argv[ix], L"/i") == 0){
                launch_minimized = true;
            }
        }
        if (ix < args.argc){
            full_path path{args.argv[ix]};
            if (*static_cast<const wchar_t*>(path) != 0){
                script_path_buf = path;
                script_path = script_path_buf.c_str();
            }
        }
    }
}
