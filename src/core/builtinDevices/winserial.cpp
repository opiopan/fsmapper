//
// winserial.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include <sstream>
#include "winserial.h"

static const auto EVIX_READ = 0;
static const auto EVIX_SIGNAL = 1;
static const auto EVIX_WRITE = 2;

WinSerial::WinSerial(const char* path): path(path), should_be_stop(false), is_writing(false), written_len(0){
    serial = ::CreateFileA(
        path,
        GENERIC_READ | GENERIC_WRITE,
        0,                            // no shared
        nullptr,                      // security attributes: default
        OPEN_EXISTING,
        FILE_FLAG_OVERLAPPED,
        nullptr                       // file template
    );
    if (serial == INVALID_HANDLE_VALUE){
        std::ostringstream os;
        os << "cannot open COM port: " << path;
        throw SimHIDConnection::Exception(os.str());
    }
    
    DCB dcb;
    ::GetCommState(serial, &dcb);
    dcb.BaudRate = 115200;
    dcb.ByteSize = 8;
    dcb.Parity = NOPARITY;
    dcb.StopBits = ONE5STOPBITS;
    dcb.fOutxCtsFlow = false;
    dcb.fRtsControl = RTS_CONTROL_DISABLE;
    dcb.EvtChar = 0;
    ::SetCommState(serial, &dcb);

    event_read = CreateEventA(nullptr,true, false, nullptr);
    event_write = CreateEventA(nullptr,true, false, nullptr);
    event_signal = CreateEventA(nullptr,true, false, nullptr);
    events[EVIX_READ] = event_read;
    events[EVIX_WRITE] = event_write;
    events[EVIX_SIGNAL] = event_signal;
    if (event_read == INVALID_HANDLE_VALUE || event_write == INVALID_HANDLE_VALUE || event_signal == INVALID_HANDLE_VALUE){
        std::ostringstream os;
        os << "failed to create event objects for COM port: " << path;
        throw SimHIDConnection::Exception(os.str());
    }

    ::PurgeComm(serial, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
}

WinSerial::~WinSerial(){
    stop();
};

int WinSerial::read(void* buf, int len){
    OVERLAPPED ov_read = {0};
    ov_read.hEvent = event_read;
    DWORD read_size;
    if (!::ReadFile(serial, buf, 1, &read_size, &ov_read)){
        if (::GetLastError() != ERROR_IO_PENDING){
            std::ostringstream os;
            os << "An error occurred when receiving data from COM port: " << path;
            throw SimHIDConnection::Exception(os.str());
        }
    }else{
        return read_size;
    }

    std::unique_lock lock(mutex);
    while (true){
        if (should_be_stop){
            return -1;
        }
        if (!is_writing && write_buf.size() > 0){
            ov_write = {0};
            ov_write.hEvent = event_write;
            auto& data = write_buf.front();
            DWORD written;
            if (!::WriteFile(serial, data.data() + written_len, data.size() - written_len, &written, &ov_write)){
                if (::GetLastError() != ERROR_IO_PENDING){
                    std::ostringstream os;
                    os << "An error occurred when sending data via COM port: " << path;
                    throw SimHIDConnection::Exception(os.str());
                }else{
                    is_writing = true;
                }
            }else{
                is_writing = false;
                written_len += written;
                if (written_len == data.size()){
                    written_len = 0;
                    write_buf.pop();
                }
            }
        }
        auto event_num = is_writing ? 3 : 2;
        lock.unlock();
        auto signaled_event = ::WaitForMultipleObjects(event_num, events, false, INFINITE);
        lock.lock();
        if (signaled_event == EVIX_SIGNAL){
            ::ResetEvent(event_signal);
        }else if (signaled_event == EVIX_READ){
            ::ResetEvent(event_read);
            if (!GetOverlappedResult(serial, &ov_read, &read_size, false)){
                std::ostringstream os;
                os << "An error occurred when receiving data from COM port: " << path;
                throw SimHIDConnection::Exception(os.str());
            }
            return read_size;
        }else if (signaled_event == EVIX_WRITE){
            ::ResetEvent(event_write);
            DWORD written;
            if (!GetOverlappedResult(serial, &ov_write, &written, false)){
                std::ostringstream os;
                os << "An error occurred when sending data via COM port: " << path;
                throw SimHIDConnection::Exception(os.str());
            }
            is_writing = false;
            written_len += written;
            if (written_len == write_buf.front().size()){
                written_len = 0;
                write_buf.pop();
            }
        }else{
            std::ostringstream os;
            os << "An error occurred when waiting until transfer is complete via COM port: " << path;
            throw SimHIDConnection::Exception(os.str());
        }
    }
}

void WinSerial::write(std::string&& data){
    std::lock_guard lock(mutex);
    write_buf.push(std::move(data));
    SetEvent(event_signal);
}

void WinSerial::stop(){
    std::lock_guard lock(mutex);
    should_be_stop = true;
    SetEvent(event_signal);
}
