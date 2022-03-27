//
// encoding.hpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once

#include <memory>
#include <windows.h>

namespace tools{
    inline int utf8_to_utf16(const char* from, wchar_t* to, int to_len){
        return ::MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, from, -1, to, to_len);
    }

    inline int utf16_to_utf8(const wchar_t* from, char* to, int to_len){
        return ::WideCharToMultiByte(CP_UTF8, 0, from, -1, to, to_len, nullptr, nullptr);
    }

    template <typename STR_FROM, typename STR_TO, int TRANSLATE(const STR_FROM*, STR_TO*, int)>
    class base_translator{
        static constexpr auto initial_buf_size = 0x40; // must be power of two
        static constexpr auto initial_buf_size_mask = ~(initial_buf_size - 1);
        static constexpr auto initial_element_num_in_int32 = initial_buf_size / (sizeof(int32_t) / sizeof(char));
        static constexpr auto initial_element_num = initial_buf_size / (sizeof(STR_TO) / sizeof(char));

        STR_TO* buf = initial_buf.data;
        int buf_len = initial_element_num;
        int str_len = 0;
        union {
            int32_t dummy[initial_element_num_in_int32];
            STR_TO data[initial_element_num];
        } initial_buf;
        std::unique_ptr<STR_TO> dynamic_buf;

    public:
        base_translator(int size = 0){
            expand_buf(size);
        }

        base_translator(const STR_FROM* str, int size = 0){
            expand_buf(size);
            translate(str);
        }

        base_translator(const base_translator&) = delete;
        base_translator(base_translator&&) = delete;
        base_translator& operator = (const base_translator&) = delete;
        base_translator& operator = (base_translator&&) = delete;

        const STR_TO* get() const noexcept {return buf;}
        operator const STR_TO* () const noexcept {return buf;}

        int size(){return str_len;}
        void clear(){str_len = 0;}

        bool translate(const STR_FROM* from){
            bool succeeded = false;
            while (!succeeded){
                str_len = TRANSLATE(from, buf, buf_len -1);
                buf[str_len] = 0;
                if (str_len == 0){
                    if (::GetLastError() == ERROR_INSUFFICIENT_BUFFER){
                        auto required_len  = TRANSLATE(from, nullptr, 0);
                        expand_buf(required_len + 1);
                    }else{
                        return false;
                    }
                }else{
                    succeeded = true;
                }
            }
            return true;
        }
        
        base_translator& operator = (const STR_FROM* from){
            translate(from);
            return *this;
        }

    protected:
        void expand_buf(int size){
            if (size > buf_len){
                auto newsize = 
                    ((size * sizeof(STR_TO) + initial_buf_size - 1) & initial_buf_size_mask) / sizeof(STR_TO);
                dynamic_buf = std::move(std::unique_ptr<STR_TO>(new STR_TO[newsize]));
                buf = dynamic_buf.get();
                buf_len = static_cast<int>(newsize);
                str_len = 0;
            }
        }
    };

    using utf8_to_utf16_translator = base_translator<char, wchar_t, utf8_to_utf16>;
    using utf16_to_utf8_translator = base_translator<wchar_t, char, utf16_to_utf8>;
}