//
// tool.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once

#include <windows.h>
#include <string.h>
#include <dcommon.h>

#ifndef NO_SOL
#include <memory>
#include <optional>
#include <algorithm>
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

class WinDC{
protected:
    HWND hwnd;
    HDC hdc;
public:
    WinDC(HWND hwnd = nullptr) : hwnd(hwnd), hdc(::GetDC(hwnd)){}
    WinDC() = delete;
    WinDC(const WinDC&) = delete;
    ~WinDC(){close();}
    WinDC& operator = (const WinDC&) = delete;
    HDC get()const {return hdc;}
    operator HDC ()const {return hdc;}
    void close(){
        if (hdc){
            ::ReleaseDC(hwnd, hdc);
            hwnd = nullptr;
            hdc = nullptr;
        }
    }
};

class MemDC{
protected:
    HDC hdc;
public:
    MemDC(HDC dc = nullptr) : hdc(::CreateCompatibleDC(dc)){}
    MemDC() = delete;
    MemDC(const MemDC&) = delete;
    ~MemDC(){close();}
    MemDC& operator = (const MemDC&) = delete;
    HDC get()const {return hdc;}
    operator HDC ()const {return hdc;}
    void close(){
        if (hdc){
            ::DeleteDC(hdc);
            hdc = nullptr;
        }
    }
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

#ifndef NO_SOL
template <typename T>
struct PointBase{
    T x = 0;
    T y = 0;

    PointBase() = default;
    PointBase(T x, T y) : x(x), y(y){}
    template <typename POINT>
    PointBase(const POINT& src){*this = src;}
    ~PointBase() = default;

    template<typename POINT>
    PointBase& operator = (const POINT& src){
        x = static_cast<T>(src.x);
        y = static_cast<T>(src.y);
        return *this;
    }

    template<typename POINT>
    PointBase& operator += (const POINT& rvalue){
        x += static_cast<T>(rvalue.x);
        y += static_cast<T>(rvalue.y);
    }

    template<typename POINT>
    PointBase operator + (const POINT& rvalue) const{
        return {x + static_cast<T>(rvalue.x), y + static_cast<T>(rvalue.y)};
    }
};

using IntPoint = PointBase<int>;
using FloatPoint = PointBase<float>;

template <typename T> 
struct RectangleBase{
    static constexpr auto zero = static_cast<T>(0);
    T x = 0;
    T y = 0;
    T width = 0;
    T height = 0;

    template <typename ANY>
    static constexpr ANY max(ANY a, ANY b){
        if (a > b){
            return a;
        }else{
            return b;
        }
    }

    template <typename ANY>
    static constexpr ANY min(ANY a, ANY b){
        if (a < b){
            return a;
        }else{
            return b;
        }
    }

    RectangleBase() = default;
    RectangleBase(T x, T y, T width, T height) : x(x), y(y), width(width), height(height){};
    ~RectangleBase() = default;
    template <typename SRC>
    RectangleBase(const SRC& src): 
        x(static_cast<T>(src.x)), y(static_cast<T>(src.y)), 
        width(static_cast<T>(src.width)), height(static_cast<T>(src.height)){}

    template<typename SRC>
    RectangleBase& operator = (const SRC& src){
        x = src.x;
        y = src.y;
        width = src.width;
        height = src.height;
        return *this;
    };

    template <typename ANY>
    RectangleBase& operator += (const PointBase<ANY>& point){
        x += static_cast<T>(point.x);
        y += static_cast<T>(point.y);
        return *this;
    }

    template <typename ANY>
    RectangleBase operator + (const PointBase<ANY>& point) const{
        return {
            x + static_cast<T>(point.x), y + static_cast<T>(point.y),
            width, height
        };
    }

    template <typename ANY>
    RectangleBase& operator += (const RectangleBase<ANY>& rect){
        if (width == zero || height == zero){
            *this = rect;
        }else{
            auto x2 = max(x + width, static_cast<T>(rect.x + rect.width));
            auto y2 = max(y + height, static_cast<T>(rect.y + rect.height));
            x = min(x, static_cast<T>(rect.x));
            y = min(y, static_cast<T>(rect.y));
            width = x2 - x;
            height = y2 - y;
        }
        return *this;
    }

    template <typename ANY>
    RectangleBase operator + (const RectangleBase<ANY>& rect) const{
        auto result = *this;
        result += rect;
        return result;
    }
    
    bool operator == (const RectangleBase& src) const{
        return x == src.x && y == src.y && width == src.width && height == src.height;
    }

    bool operator != (const RectangleBase& src) const{
        return !(*this == src);
    }

    operator D2D1_RECT_F () const{
        return D2D1_RECT_F{
            static_cast<float>(x),
            static_cast<float>(y),
            static_cast<float>(x + width),
            static_cast<float>(y + height)
        };
    }

    bool pointIsInRectangle(T tx, T ty) const{
        return tx >= x && tx <= x + width &&
               ty >= y && ty <= y + height;
    }

    RectangleBase intersect(const RectangleBase& src) const{
        auto right = x + width;
        auto bottom = y + height;
        auto src_right = src.x + src.width;
        auto src_bottom = src.y + src.height;
        auto new_x = std::max(x, src.x);
        auto new_y = std::max(y, src.y);
        auto new_right = std::min(right, src_right);
        auto new_bottom = std::min(bottom, src_bottom);
        return {new_x,
                new_y,
                std::max(0, new_right - new_x),
                std::max(0, new_bottom - new_y)};
    }
};

using IntRect = RectangleBase<int>;
using FloatRect = RectangleBase<float>;
#endif

extern std::optional<COLORREF> webcolor_to_colorref(const std::string& color_str);