//
// winserial.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once
#include <windows.h>
#include <queue>
#include "simhidconnection.h"

class WinSerial : public SimHIDConnection::SerialComm{
protected:
    class WinHandle{
    protected:
        HANDLE handle;
    public:
        WinHandle():handle(INVALID_HANDLE_VALUE){};
        WinHandle(HANDLE handle):handle(handle){};
        WinHandle(const WinHandle&) = delete;
        WinHandle(WinHandle&&) = delete;
        ~WinHandle(){if (handle != INVALID_HANDLE_VALUE){CloseHandle(handle);}};
        WinHandle& operator = (WinHandle&) = delete;
        WinHandle& operator = (WinHandle&&) = delete;
        WinHandle& operator = (HANDLE handle){
            if (this->handle != INVALID_HANDLE_VALUE){
                CloseHandle(handle);
            }
            this->handle = handle;
            return *this;
        };
        HANDLE get_handle()const {return handle;};
        operator HANDLE()const {return handle;};
    };

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
