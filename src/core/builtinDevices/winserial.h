//
// winserial.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once
#define NOMINMAX
#include <windows.h>
#include <queue>
#include "simhidconnection.h"
#include "tools.h"

class WinSerial : public SimHIDConnection::SerialComm{
protected:
    std::string path;
    std::mutex mutex;
    bool should_be_stop;
    bool is_writing;
    std::queue<std::string>write_buf;
    size_t written_len;
    WinHandle serial;
    WinHandle event_write;
    WinHandle event_read;
    WinHandle event_signal;
    HANDLE events[3];
    OVERLAPPED ov_write;
public:
    WinSerial() = delete;
    WinSerial(const WinSerial&) = delete;
    WinSerial(WinSerial&&) = delete;
    WinSerial(const char* path);
    virtual ~WinSerial();
    virtual int read(void* buf, int len);
    virtual void write(std::string&& data);
    virtual void stop();
};
