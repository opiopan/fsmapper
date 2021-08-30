//
// tool.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once

#include <windows.h>
#include <string.h>

#ifndef NO_SOL
#include <memory>
#include <optional>
#include <sol/sol.hpp>

inline std::string lua_safestring(const sol::object& object){
    if (object.get_type() == sol::type::string){
        return object.as<std::string>();
    }else{
        return std::move(std::string());
    }
};

template <typename T>
std::optional<T> lua_safevalue(const sol::object& object){
    using rtype = std::optional<T>;
    if (object.get_type() == sol::type::lua_nil){
        return rtype();
    }else if (object.is<T>()){
        return std::move(rtype(object.as<T>()));
    }else{
        return rtype();
    }
}
#endif

class WinHandle{
protected:
    HANDLE handle = INVALID_HANDLE_VALUE;
public:
    WinHandle() = default;
    WinHandle(HANDLE handle):handle(handle){};
    WinHandle(const WinHandle&) = delete;
    WinHandle(WinHandle&& src) noexcept{*this = std::move(src);};
    ~WinHandle(){
        if (handle != INVALID_HANDLE_VALUE && handle != nullptr){
            CloseHandle(handle);
        }
    };
    WinHandle& operator = (HANDLE handle){
        if (this->handle != INVALID_HANDLE_VALUE && handle != nullptr){
            CloseHandle(this->handle);
        }
        this->handle = handle;
        return *this;
    };
    WinHandle& operator = (WinHandle&) = delete;
    WinHandle& operator = (WinHandle&& src){
        *this = src.handle;
        src.handle = INVALID_HANDLE_VALUE;
        return *this;
    };
    HANDLE get_handle()const {return handle;};
    operator HANDLE()const {return handle;};
};

template <typename T>
class GdiObject{
protected:
    T object = nullptr;
public:
    GdiObject() = default;
    GdiObject(T object): object(object){};
    GdiObject(const GdiObject&) = delete;
    GdiObject(GdiObject&& src) noexcept {*this = std::move(src);};
    ~GdiObject(){
        if (object){
            ::DeleteObject(object);
        }
    }
    GdiObject& operator = (T object){
        if (this->object){
            ::DeleteObject(this->object);
        }
        this->object = object;
        return *this;
    }
    GdiObject& operator = (const GdiObject&) = delete;
    GdiObject& operator = (GdiObject&& src){
        *this = src.object;
        src.object = nullptr;
        return *this;
    }
    T get_handle()const {return object;};
    operator T ()const {return object;};
};

template <typename T>
class ComPtr{
protected:
    T* com = nullptr;
public:
    ComPtr() = default;
    ComPtr(T* com) : com(com){};
    ComPtr(const ComPtr& src) noexcept {
        *this = src;
    };
    ComPtr(ComPtr&& src) noexcept {
        *this = std::move(src);
    };
    ~ComPtr(){
        if (com){
            com->Release();
        }
    }
    ComPtr& operator = (T* com){
        this->attach(com);
        return *this;
    }
    ComPtr& operator = (const ComPtr& src){
        *this = src.com;
        if (com){
            com->AddRef();
        }
        return *this;
    }
    ComPtr& operator = (ComPtr&& src){
        *this = src.com;
        src.com = nullptr;
        return *this;
    }
    void attach(T* com){
        if (this->com){
            this->com->Release();
        }
        this->com = com;
    }
    T* detach(){
        auto rc = com;
        com = nullptr;
        return rc;
    }
    operator T* ()const {return com;};
    T* operator -> ()const {return com;};
};

struct GUID_KEY: public GUID{
    GUID_KEY(const GUID& guid) : GUID(guid){};
    GUID_KEY(const GUID_KEY&) = default;
    ~GUID_KEY() = default;
    int compare(const GUID_KEY& v) const{
        return memcmp(this, &v, sizeof(*this));
    }
    bool operator < (const GUID_KEY& rval) const{
        return compare(rval) < 0;
    }
    bool operator > (const GUID_KEY& rval) const{
        return compare(rval) > 0;
    }
    bool operator == (const GUID_KEY& rval) const{
        return compare(rval) == 0;
    }
};

extern std::optional<COLORREF> webcolor_to_colorref(const std::string& color_str);