//
// posixserial.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include <fcntl.h>
#include <termios.h>
#include <strings.h>

#include <unistd.h>

#include <memory>
#include <sstream>
#include "posixserial.h"

static const auto COMMSPEED = B115200;

PosixSerial::PosixSerial(const char *path) : should_be_stop(false), written_len(0){
    // cofigure serial port
    fd_serial = open(path, O_RDWR | O_NONBLOCK);
    if (fd_serial < 0){
        auto err = strerror(errno);
        std::ostringstream os;
        os << "cannot open file: " << err << ": " << path;
        throw SimHIDConnection::Exception(os.str());
    }
    termios tio;
    bzero(&tio, sizeof(tio));
    tio.c_cflag = COMMSPEED | CRTSCTS | CS8 | CLOCAL | CREAD;
    tio.c_iflag = IGNPAR;
    tio.c_oflag = 0;
    tio.c_cc[VTIME] = 0;
    tio.c_cc[VMIN] = 1;
    tcflush(fd_serial, TCIFLUSH);
    tcsetattr(fd_serial, TCSANOW, &tio);

    // prepare pipe to transmit system event
    int pfds[2];
    if (pipe(pfds) != 0){
        throw SimHIDConnection::Exception("An error occurred during initializing serial port");
    }
    fd_pipe_in = pfds[0];
    fd_pipe_out = pfds[1];

    // configure descriptors for poll(2)
    bzero(pollfds, sizeof(pollfds));
    pollfds[0].fd = fd_serial;
    pollfds[1].fd = fd_pipe_in;
    pollfds[1].events = POLLIN;
}

PosixSerial::~PosixSerial(){
    stop();
}
int PosixSerial::read(void* buf, int len){
    while (true){
        {
            std::lock_guard lock(mutex);
            pollfds[0].events = write_buf.size() == 0 ? POLLIN : POLLIN | POLLOUT;
            pollfds[0].revents = 0;
        }
        if (::poll(pollfds, 2, 0) < -1){
            // may reach here by only EINTR
            continue;
        }
        if (pollfds[1].revents & POLLIN){
            pollfds[1].revents = 0;
            ::read(fd_pipe_in, pipebuf, sizeof(pipebuf));
            std::lock_guard lock(mutex);
            if (should_be_stop){
                // got message to stop serial communication
                return -1;
            }
        }
        if (pollfds[0].revents & POLLOUT){
            // can send data via serial port
            std::lock_guard lock(mutex);
            auto& data = write_buf.front();
            auto result = ::write(fd_serial, data.data(), data.size() - written_len);
            if (result < 0){
                throw SimHIDConnection::Exception("Cannot send data to serial port");
            }
            written_len += result;
            if (data.size() == written_len){
                written_len = 0;
                write_buf.pop();
            }
        }
        if (pollfds[0].revents & POLLIN){
            // any data can be read from serial port
            auto result = ::read(fd_serial, buf, len);
            if (result < 0){
                throw SimHIDConnection::Exception("An error occurred when receiving data from seriral port");
            }
            return result;
        }
    }
}

void PosixSerial::write(std::string &&data){
    std::lock_guard lock(mutex);
    static char buf[1] = {1};
    write_buf.push(std::move(data));
    ::write(fd_pipe_out, buf, 1);
}

void PosixSerial::stop(){
    std::lock_guard lock(mutex);
    if (!should_be_stop){
        should_be_stop = true;
        static char buf[1] = {2};
        ::write(fd_pipe_out, buf, 1);
    }
}
