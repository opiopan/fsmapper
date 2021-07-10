//
// posixserial.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once
#include <poll.h>
#include <unistd.h>
#include <mutex>
#include <queue>
#include <string>
#include "simhidconnection.h"

class PosixSerial : public SimHIDConnection::SerialComm{
protected:
    class FD{
    protected:
        int fd;
    public:
        FD():fd(-1){};
        FD(int fd):fd(fd){};
        FD(FD&) = delete;
        FD(FD&&) = delete;
        ~FD(){if (fd >= 0){close(fd);}};
        FD& operator = (FD&) = delete;
        FD& operator = (FD&&) = delete;
        FD& operator = (int fd){
            if (fd >= 0){
                close(fd);
            }
            this->fd = fd;
            return *this;
        };
        int get_fd()const {return fd;};
        operator int()const {return fd;};
    };

    std::mutex mutex;
    FD fd_serial;
    FD fd_pipe_in;
    FD fd_pipe_out;
    bool should_be_stop;
    std::queue<std::string> write_buf;
    size_t written_len;
    pollfd pollfds[2];

public:
    PosixSerial() = delete;
    PosixSerial(PosixSerial&) = delete;
    PosixSerial(PosixSerial&&) = delete;
    PosixSerial(const char* path);
    virtual ~PosixSerial();
    virtual size_t read(void* buf, size_t len);
    virtual void write(std::string&& data);
    virtual void stop();
};
