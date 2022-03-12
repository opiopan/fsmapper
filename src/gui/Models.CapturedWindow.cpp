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
            os << "Captured: HWND = 0x"
               << std::hex << std::setw(8) << std::setfill(L'0')
               << reinterpret_cast<uint64_t>(hwnd);
        }else{
            os << "Not captured.\nClick to capture the target.";
        }
        update_property(status_string, std::move(hstring(os.str())), L"StatusString");
    }

    winrt::Windows::Foundation::IAsyncAction CapturedWindow::ToggleCapture(
        winrt::Windows::Foundation::IInspectable sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs args){
        // auto picker = winrt::Windows::Graphics::Capture::GraphicsCapturePicker();
        // picker.as<IInitializeWithWindow>()->Initialize(App::TopWindowHandle());
        // auto item = co_await picker.PickSingleItemAsync();
        // if (item) {
        // }

        // auto dialog = winrt::Microsoft::UI::Xaml::Controls::ContentDialog();
        // dialog.Title(winrt::box_value(L"Test dialog"));
        // dialog.Content(winrt::box_value(L"This is test of ContentDialog on WinRT/C++."));
        // dialog.CloseButtonText(L"Close");
        // dialog.XamlRoot(App::TopWindow().Content().XamlRoot());
        // auto result = co_await dialog.ShowAsync();

        auto result = co_await winrt::gui::PickWindowAsync(name);
    }

}
