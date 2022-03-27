//
// encoding.hpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once

#include <stdexcept>
#include <combaseapi.h>
#include "encoding.hpp"

namespace tools {
    class guid{
        GUID id;
        utf16_to_utf8_translator str;
        LPOLESTR olestr{nullptr};

    public:
        guid(): id{0}{}
        guid(const GUID& id): id{id}{}
        guid(const char* str){*this = str;}
        guid(const wchar_t* str){*this = str;}
        guid(const guid& src): id{src.id}{}
        ~guid(){reset();}

        guid& operator = (const GUID& id){
            this->id = id;
            reset();
            return *this;
        }
        guid& operator = (const guid& src){
            id = src.id;
            reset();
            return *this;
        }
        guid& operator = (const wchar_t* str){
            if (::CLSIDFromString(str, &id) != NOERROR){
                throw std::runtime_error("GUID string format may be invalid");
            }
            reset();
            return *this;
        }
        guid& operator = (const char* str){
            utf8_to_utf16_translator utf16_str{str};
            *this = utf16_str;
            return *this;
        }

        operator const GUID& () const noexcept{
            return id;
        }

        operator const wchar_t* (){
            if (!olestr && ::StringFromCLSID(id, &olestr) != NOERROR){
                throw std::runtime_error("system error: not enough memory");
            }
            return olestr;
        }

        operator const char* (){
            str.translate(*this);
            return str;
        }

    protected:
        void reset(){
            str.clear();
            if (olestr){
                ::CoTaskMemFree(olestr);
                olestr = nullptr;
            }
        }
    };
}