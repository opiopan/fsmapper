//
// Models.CapturedWindow.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//
#include "pch.h"
#include "Models.h"
#include "Models.CapturedWindow.g.cpp"

#include <winrt/Microsoft.UI.Interop.h>

#include <sstream>
#include <iomanip>

namespace winrt::gui::Models::implementation{
    void CapturedWindow::update_status_string(){
        std::wostringstream os;
        if (is_captured) {
            auto hwnd = GetWindowFromWindowId(window_id);
            os << "Captured: HWND = 0x"
               << std::hex << std::setw(8) << std::setfill(L'0')
               << reinterpret_cast<uint64_t>(hwnd);
        }else{
            os << "Not captured.\nPush to capture the target.";
        }
        update_property(status_string, std::move(hstring(os.str())), L"StatusString");
    }

    winrt::Windows::Foundation::IAsyncAction CapturedWindow::ToggleCapture(
        winrt::Windows::Foundation::IInspectable sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs args){
        winrt::apartment_context ui_thread;
        co_await winrt::resume_background();
        co_await ui_thread;
    }

}
