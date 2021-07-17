//
// winserial.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once
#include "simhidconnection.h"

class WinSerial : public SimHIDConnection::SerialComm{
protected:

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
