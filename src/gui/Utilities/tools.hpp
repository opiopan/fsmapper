//
// tools.hpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once

#include <winrt/Windows.Foundation.h>
#include <winrt/Microsoft.UI.Xaml.h>

namespace tools{
    inline winrt::Windows::Foundation::IInspectable AppResource(const wchar_t* name){
        return winrt::Microsoft::UI::Xaml::Application::Current().Resources().Lookup(winrt::box_value(name));
    }

    inline winrt::Windows::Foundation::IInspectable ThemeResource(const wchar_t* name){
        auto theme = winrt::Microsoft::UI::Xaml::Application::Current().RequestedTheme();
        auto theme_name = theme == winrt::Microsoft::UI::Xaml::ApplicationTheme::Dark ? L"Dark" : L"Light";
        auto udict = winrt::Microsoft::UI::Xaml::Application::Current().Resources().ThemeDictionaries().Lookup(winrt::box_value(theme_name));
        auto dict = udict.as<winrt::Microsoft::UI::Xaml::ResourceDictionary>();
        return dict.Lookup(winrt::box_value(name));
    }

    template <int APIFUNC(HWND, LPWSTR, int)>
    class safe_text_api{
        static constexpr auto incremental_amount = 256;
        std::vector<wchar_t> buf;

    public:
        safe_text_api(){
            buf.resize(incremental_amount);
        }
        const wchar_t * operator ()(HWND hwnd){
            while (true){
                auto rc = APIFUNC(hwnd, &buf.at(0), static_cast<int>(buf.size()));
                if (rc < buf.size() - 1){
                    return &buf.at(0);
                }else{
                    buf.resize(buf.max_size() + incremental_amount);
                }
            }
        }
    };

    class win_handle{
    protected:
        HANDLE handle = INVALID_HANDLE_VALUE;
    public:
        win_handle() = default;
        win_handle(HANDLE handle):handle(handle){};
        win_handle(const win_handle&) = delete;
        win_handle(win_handle&& src) noexcept{*this = std::move(src);};
        ~win_handle(){
            if (handle != INVALID_HANDLE_VALUE && handle != nullptr){
                CloseHandle(handle);
            }
        };
        win_handle& operator = (HANDLE new_handle){
            if (this->handle != INVALID_HANDLE_VALUE && new_handle != nullptr){
                CloseHandle(this->handle);
            }
            this->handle = new_handle;
            return *this;
        };
        win_handle& operator = (win_handle&) = delete;
        win_handle& operator = (win_handle&& src){
            *this = src.handle;
            src.handle = INVALID_HANDLE_VALUE;
            return *this;
        };
        HANDLE get_handle()const {return handle;};
        operator HANDLE()const {return handle;};
    };

    template <typename T>
    class gdi_object{
    protected:
        T object = nullptr;
    public:
        gdi_object() = default;
        gdi_object(T object): object(object){};
        gdi_object(const gdi_object&) = delete;
        gdi_object(gdi_object&& src) noexcept {*this = std::move(src);};
        ~gdi_object(){
            if (object){
                ::DeleteObject(object);
            }
        }
        gdi_object& operator = (T object){
            if (this->object){
                ::DeleteObject(this->object);
            }
            this->object = object;
            return *this;
        }
        gdi_object& operator = (const gdi_object&) = delete;
        gdi_object& operator = (gdi_object&& src){
            *this = src.object;
            src.object = nullptr;
            return *this;
        }
        T get_handle()const {return object;};
        operator T ()const {return object;};
    };

    class win_dc{
    protected:
        HWND hwnd;
        HDC hdc;
    public:
        win_dc(HWND hwnd = nullptr) : hwnd(hwnd), hdc(::GetDC(hwnd)){}
        win_dc() = delete;
        win_dc(const win_dc&) = delete;
        ~win_dc(){close();}
        win_dc& operator = (const win_dc&) = delete;
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

    class mem_dc{
    protected:
        HDC hdc;
    public:
        mem_dc(HDC dc = nullptr) : hdc(::CreateCompatibleDC(dc)){}
        mem_dc() = delete;
        mem_dc(const mem_dc&) = delete;
        ~mem_dc(){close();}
        mem_dc& operator = (const mem_dc&) = delete;
        HDC get()const {return hdc;}
        operator HDC ()const {return hdc;}
        void close(){
            if (hdc){
                ::DeleteDC(hdc);
                hdc = nullptr;
            }
        }
    };
   
}