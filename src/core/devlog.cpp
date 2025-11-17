//
// devog.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include "devlog.h"

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

class logger_impl : public devlog::logger {
protected:
    std::mutex mutex;
    std::ofstream ofs;
    
public:
    logger_impl(){
        // Retrieve AppData\Roaming path
        wchar_t* roaming;
        SHGetKnownFolderPath(FOLDERID_RoamingAppData, KF_FLAG_DEFAULT, nullptr, &roaming);
        std::filesystem::path roaming_dir{roaming};
        CoTaskMemFree(roaming);

        // Construct log file path
        auto&& logpath = roaming_dir / "fsmapper" / "logs" / "fsmapper.log";

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

    ~logger_impl(){
        ofs.close();
    }

    void log(MCONSOLE_MESSAGE_TYPE type, const char* message) override {
        auto now = std::chrono::system_clock::now();
        auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

        auto type_str = type == MCONSOLE_ERROR ? "ERROR" :
                        type == MCONSOLE_WARNING ? "WARNING" :
                        type == MCONSOLE_INFO ? "INFO" :
                        type == MCONSOLE_MESSAGE ? "MESSAGE" :
                        type == MCONSOLE_DEBUG ? "DEBUG" :
                        type == MCONSOLE_EVENT ? "EVENT" : "UNKNOWN";

        std::lock_guard lock(mutex);
        ofs << std::format(
            "[{:%Y-%m-%d %H:%M:%S}.{:03}] [{}] {}\n",
            std::chrono::floor<std::chrono::seconds>(now),
            now_ms.count(),
            type_str,
            message);
        ofs.flush();
    }
};

class null_logger_impl : public devlog::logger {
public:
    ~null_logger_impl() {}
    void log(MCONSOLE_MESSAGE_TYPE, const char*) override {}
};

}

std::unique_ptr<devlog::logger> devlog::make_logger(bool to_file) {
    if (to_file) {
        return std::make_unique<logger_impl>();
    } else {
        return std::make_unique<null_logger_impl>();
    }
}
