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
}