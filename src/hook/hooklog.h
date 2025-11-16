//
// hooklog.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once
#include <string>

namespace hooklog {
    class logger {
    public:
        virtual void log(const char* message) = 0;
        void log(const std::string& message){
            log(message.c_str());
        }
    };

    void allocate_logger();
    void release_logger();
    logger& get_logger();
}
