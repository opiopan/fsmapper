//
// hooklog.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include "hooklog.h"

#include <memory>
#include <mutex>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <format>
#include <exception>

#include <windows.h>
#include <Shlobj.h>

namespace {

class logger_impl : public hooklog::logger {
protected:
    std::mutex mutex;
    std::ofstream ofs;
    
public:
    logger_impl(){
        // Retrieve module path for this process
        char modpath_str[MAX_PATH];
        DWORD modpath_len = sizeof(modpath_str);
        GetModuleFileNameA(nullptr, modpath_str, modpath_len);
        std::filesystem::path modpath{modpath_str};

        // Retrieve AppData\Roaming path
        wchar_t* roaming;
        SHGetKnownFolderPath(FOLDERID_RoamingAppData, KF_FLAG_DEFAULT, nullptr, &roaming);
        std::filesystem::path roaming_dir{roaming};
        CoTaskMemFree(roaming);

        // Construct log file path
        auto&& logname = std::format("hook_{}.log", modpath.stem().string());
        auto&& logpath = roaming_dir / "fsmapper" / "logs" / logname;

        // Ensure directory exists
        if (logpath.has_parent_path()) {
            std::filesystem::create_directories(logpath.parent_path());
        }

        // Open log file
        ofs.open(logpath, std::ios::out | std::ios::trunc);
        if (!ofs.is_open()) {
            throw std::runtime_error("Failed to open log file: " + logpath.string());
        }
    }

    void log(const char* message) override {
        auto now = std::chrono::system_clock::now();
        auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

        std::lock_guard lock(mutex);
        ofs << std::format(
            "[{:%Y-%m-%d %H:%M:%S}.{:03}] {}\n", 
            std::chrono::floor<std::chrono::seconds>(now), 
            now_ms.count(),
            message);
        ofs.flush();
    }
};

class null_logger_impl : public hooklog::logger {
public:
    void log(const char* message) override {}
};

}

std::unique_ptr<hooklog::logger> global_logger;
auto logger_count{0};
null_logger_impl null_logger;

void hooklog::allocate_logger() {
    try{
        if (logger_count == 0) {
            global_logger = std::make_unique<logger_impl>();
        }
        ++logger_count;
    }catch(std::runtime_error&){
        // Failed to create logger, keep it null
    }
}

void hooklog::release_logger() {
    if (logger_count > 0) {
        --logger_count;
        if (logger_count == 0) {
            global_logger.reset();
        }
    }
}

hooklog::logger& hooklog::get_logger() {
    if (global_logger) {
        return *global_logger;
    }else{
        return null_logger;
    }
}