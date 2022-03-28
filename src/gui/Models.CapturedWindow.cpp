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

//#include <shobjidl.h>
#include <sstream>
#include <iomanip>

#include <dwmapi.h>
#include <winrt/Microsoft.Graphics.Canvas.h>
#include <winrt/Microsoft.Graphics.Canvas.UI.Xaml.h>
#include <winrt/Microsoft.Graphics.DirectX.h>
#include <winrt/Windows.Graphics.Capture.h>
#include <Windows.Graphics.Capture.Interop.h>
#include <winrt/Microsoft.UI.Interop.h>
#include <winrt/Windows.Graphics.Imaging.h>
#include <winrt/Microsoft.UI.Xaml.Media.Imaging.h>

using App = winrt::gui::implementation::App;

namespace winrt::gui::Models::implementation{
    //============================================================================================
    // property implementations
    //    These propety should not be implemented in the model object,
    //    but they should be implemented as converter.
    //    There is no choice but to do this since current WinUI3 & WinRT/C++ toolchain has a
    //    problem to handle IValueConverter. (as of March 2022)
    //============================================================================================
    winrt::Microsoft::UI::Xaml::Media::SolidColorBrush CapturedWindow::ButtonTitleColor(){
        auto key = is_captured ? L"CapturedWindowButtonTitleCaptured" : L"CapturedWindowButtonTitleUncaptured";
        auto value = tools::ThemeResource(key);
        return value.as<winrt::Microsoft::UI::Xaml::Media::SolidColorBrush>();
    }

    winrt::Microsoft::UI::Xaml::Media::SolidColorBrush CapturedWindow::ButtonTextColor(){
        auto key = is_captured ? L"CapturedWindowButtonTextCaptured" : L"CapturedWindowButtonTextUncaptured";
        auto value = tools::ThemeResource(key);
        return value.as<winrt::Microsoft::UI::Xaml::Media::SolidColorBrush>();
    }

    //============================================================================================
    // property update in order to according to captured status change
    //============================================================================================
    void CapturedWindow::reflect_status_change(bool captured_state){
        update_property(is_captured, captured_state, L"IsCaptured");
        std::wostringstream os;
        if (is_captured) {
            os << L"Captured: HWND = 0x"
               << std::hex << std::setw(8) << std::setfill(L'0')
               << window_id;
        }else{
            os << L"Not captured.\nClick to capture the target.";
            if (auto strong_mapper{mapper.get()}){
                update_property(image, strong_mapper.NullWindowImage(), L"Image");
            }
        }
        update_property(status_string, std::move(hstring(os.str())), L"StatusString");
        update_property(L"ButtonTitleColor");
        update_property(L"ButtonTextColor");
    }

    //============================================================================================
    // capture / release window
    //============================================================================================
    winrt::Windows::Foundation::IAsyncAction CapturedWindow::ToggleCapture(
        winrt::Windows::Foundation::IInspectable, winrt::Microsoft::UI::Xaml::RoutedEventArgs){
        if (is_captured){
            winrt::Microsoft::UI::Xaml::Controls::ContentDialog dialog;
            dialog.Title(winrt::box_value(L"Release Captured Window"));
            std::wostringstream os;
            os << L"Are you sure you want to release the captured window for \"";
            os << name.c_str() << L"\" ?";
            dialog.Content(winrt::box_value(hstring(os.str())));
            dialog.PrimaryButtonText(L"Yes");
            dialog.CloseButtonText(L"No");
            dialog.XamlRoot(App::TopWindow().Content().XamlRoot());
            auto result =  co_await dialog.ShowAsync();

            if (result == winrt::Microsoft::UI::Xaml::Controls::ContentDialogResult::Primary){
                window_id = 0;
                do_release_window();
            }
            co_return;
        }else{
            #if 0
            auto result = co_await winrt::gui::PickWindowAsync(name);
            auto hwnd = result ? reinterpret_cast<HWND>(result.hWnd()) : nullptr;
            #else
            winrt::apartment_context ui_thread;
            auto appwnd = App::TopWindowHandle();
            tools::utf16_to_utf8_translator target_name(name.c_str());
            ::ShowWindow(appwnd, SW_HIDE);
            co_await winrt::resume_background();
            auto hwnd = mapper_tools_PickWindow(appwnd, target_name);
            co_await ui_thread;
            ::ShowWindow(appwnd, SW_SHOW);
            #endif
            auto strong_mapper{mapper.get()};
            if (strong_mapper && hwnd){
                window_id = reinterpret_cast<uint64_t>(hwnd);
                prepare_capture();
                start_capture();
            }
        }
    }

    void CapturedWindow::do_capture_window(){
        if (auto strong_mapper{mapper.get()}){
            strong_mapper.CaptureWindow(cwid, window_id);
            reflect_status_change(true);
        }
    }

    void CapturedWindow::do_release_window(){
        if (auto strong_mapper{mapper.get()}){
            strong_mapper.ReleaseWindow(cwid);
            reflect_status_change(false);
        }
    }


    //============================================================================================
    // Window image capturing
    //============================================================================================
    void CapturedWindow::prepare_capture(){
        const auto factory = get_activation_factory<winrt::Windows::Graphics::Capture::GraphicsCaptureItem>();
        const auto interop = factory.as<IGraphicsCaptureItemInterop>();
        try{
			interop->CreateForWindow(
				reinterpret_cast<HWND>(window_id),
				winrt::guid_of<ABI::Windows::Graphics::Capture::IGraphicsCaptureItem>(),
				reinterpret_cast<void**>(winrt::put_abi(capturing.item)));
        }catch(...){
        }
    }

    void CapturedWindow::start_capture(){
        if (capturing.item){
            capturing.frame_pool = winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool::Create(
                capturing.canvas_device,
                winrt::Windows::Graphics::DirectX::DirectXPixelFormat::B8G8R8A8UIntNormalized,
                2, capturing.item.Size());
            
            winrt::gui::Models::CapturedWindow self{*this};
            winrt::weak_ref<winrt::gui::Models::CapturedWindow> weak_self = winrt::make_weak(self);
            capturing.token = capturing.frame_pool.FrameArrived([this, weak_self](auto const&, auto const&){
                if (weak_self.get()){
                    process_captured_frame(weak_self);
                }
            });
            capturing.session = capturing.frame_pool.CreateCaptureSession(capturing.item);
            capturing.session.IsCursorCaptureEnabled(false);
            capturing.session.StartCapture();
        }else{
            do_capture_window();
        }
    }

    void CapturedWindow::stop_capture(){
        if (capturing.session) {
            capturing.session.Close();
        }
        if (capturing.frame_pool) {
            capturing.frame_pool.FrameArrived(capturing.token);
            capturing.frame_pool.Close();
        }
        capturing.item = nullptr;
        capturing.frame_pool = nullptr;
        capturing.session = nullptr;
    }

    winrt::Windows::Foundation::IAsyncAction CapturedWindow::process_captured_frame(
        winrt::weak_ref<winrt::gui::Models::CapturedWindow> weak_self){
        if (capturing.frame_pool) {
            auto frame = capturing.frame_pool.TryGetNextFrame();
            auto surface = frame.Surface();
            auto bitmap = co_await winrt::Windows::Graphics::Imaging::SoftwareBitmap::CreateCopyFromSurfaceAsync(surface);
            if (bitmap && weak_self.get()) {
                auto displayable_image = winrt::Windows::Graphics::Imaging::SoftwareBitmap::Convert(
                    bitmap,
                    winrt::Windows::Graphics::Imaging::BitmapPixelFormat::Bgra8,
                    winrt::Windows::Graphics::Imaging::BitmapAlphaMode::Premultiplied);
                if (displayable_image) {
                    auto source = winrt::Microsoft::UI::Xaml::Media::Imaging::SoftwareBitmapSource();
                    co_await source.SetBitmapAsync(displayable_image);
                    if (weak_self.get()){
                        image = source;
                        update_property(L"Image");
                    }
                }
            }
            stop_capture();
            do_capture_window();
        }
    }
}
