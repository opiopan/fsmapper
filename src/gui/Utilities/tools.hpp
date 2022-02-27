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
}