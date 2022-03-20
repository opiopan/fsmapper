//
// Models.CapturedWindow.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//
#include "pch.h"
#include "Models.h"
#include "Models.CapturedWindow.g.cpp"

#include "App.xaml.h"
#include "tools.hpp"
#include "WindowPickerDialog.h"
#include "mappercore.h"

#include <shobjidl.h>
#include <winrt/Microsoft.UI.Interop.h>
#include <winrt/Windows.Graphics.Capture.h>
#include <sstream>
#include <iomanip>

using App = winrt::gui::implementation::App;

namespace winrt::gui::Models::implementation{
    void CapturedWindow::update_status_string(){
        std::wostringstream os;
        if (is_captured) {
            auto hwnd = GetWindowFromWindowId(window_id);
            os << L"Captured: HWND = 0x"
               << std::hex << std::setw(8) << std::setfill(L'0')
               << reinterpret_cast<uint64_t>(hwnd);
        }else{
            os << L"Not captured.\nClick to capture the target.";
        }
        update_property(status_string, std::move(hstring(os.str())), L"StatusString");
    }

    winrt::Windows::Foundation::IAsyncAction CapturedWindow::ToggleCapture(
        winrt::Windows::Foundation::IInspectable, winrt::Microsoft::UI::Xaml::RoutedEventArgs){
        // auto result = co_await winrt::gui::PickWindowAsync(name);
        // if (result) {
        //     auto name = result.Name();
        // }
        winrt::apartment_context ui_thread;
        auto appwnd = App::TopWindowHandle();
        //::ShowWindow(appwnd, SW_HIDE);
        co_await winrt::resume_background();
        auto hwnd = mapper_tools_PickWindow(appwnd);
        co_await ui_thread;
        //::ShowWindow(appwnd, SW_SHOW);
    }

}
