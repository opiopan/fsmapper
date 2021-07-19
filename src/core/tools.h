//
// tool.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include <windows.h>
#include <sol/sol.hpp>

inline std::string lua_safestring(const sol::object& object){
    if (object.get_type() == sol::type::string){
        return object.as<std::string>();
    }else{
        return std::string();
    }
};

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
            CloseHandle(this->handle);
        }
        this->handle = handle;
        return *this;
    };
    HANDLE get_handle()const {return handle;};
    operator HANDLE()const {return handle;};
};
