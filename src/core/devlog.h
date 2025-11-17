//
// devlog.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once
#include <string>
#include <memory>

#include "mappercore.h"

namespace devlog {
    class logger {
    public:
        virtual ~logger() = default;
        virtual void log(MCONSOLE_MESSAGE_TYPE type, const char *message) = 0;
        void log(MCONSOLE_MESSAGE_TYPE type, const std::string &message)
        {
            log(type, message.c_str());
        }
    };

    std::unique_ptr<logger> make_logger(bool to_file);
}
