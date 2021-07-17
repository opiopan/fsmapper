//
// winserial.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include "winserial.h"

WinSerial::WinSerial(const char* path){
}

WinSerial::~WinSerial(){

};

int WinSerial::read(void* buf, int len){
    return 0;
}

void WinSerial::write(std::string&& data){
}

void WinSerial::stop(){
}
