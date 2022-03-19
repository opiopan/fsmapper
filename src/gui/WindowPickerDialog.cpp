//
// WindowPickerDialog.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//
#include "pch.h"
#include "WindowPickerDialog.h"
#include "WindowItem.g.cpp"
#include "WindowPickerViewModel.g.cpp"
#include "WindowPickerDialog.g.cpp"
#include "App.xaml.h"

#include <string>
#include <sstream>
#include <memory>
#include <vector>

#include <dwmapi.h>
#include <winrt/Microsoft.Graphics.Canvas.h>
#include <winrt/Microsoft.Graphics.Canvas.UI.Xaml.h>
#include <winrt/Microsoft.Graphics.DirectX.h>
#include <Windows.Graphics.Capture.Interop.h>
#include <winrt/Microsoft.UI.Interop.h>
#include <winrt/Windows.Graphics.Imaging.h>
#include <winrt/Microsoft.UI.Xaml.Media.Imaging.h>


constexpr auto MINIMUM_ENABLE_WINDOW_SIZE = 64;
constexpr auto MAX_ENABLE_WINDOW_RATIO = 20;
constexpr auto DOUBLE_CLICK_INTERVAL = std::chrono::microseconds(400);

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using App = winrt::gui::implementation::App;

namespace winrt::gui{
    //============================================================================================
    // Pincking entry function
    //============================================================================================
    winrt::Windows::Foundation::IAsyncOperation<winrt::gui::WindowItem> PickWindowAsync(hstring const& target){
        auto picker = winrt::make<winrt::gui::implementation::WindowPickerViewModel>(target);
        auto window = co_await picker.PickWindowAsync();
        co_return window;
    }
}

namespace winrt::gui::implementation{
    //============================================================================================
    // constructor / destructor of WindowPickerViewModel
    //============================================================================================
    WindowPickerViewModel::WindowPickerViewModel(hstring const& target){
        window_items = winrt::single_threaded_observable_vector<gui::WindowItem>();
        auto device = winrt::Microsoft::Graphics::Canvas::CanvasDevice::GetSharedDevice();
        auto source = winrt::Microsoft::Graphics::Canvas::UI::Xaml::CanvasImageSource(device, 200, 150, 96);
        auto ds = source.CreateDrawingSession(winrt::Microsoft::UI::Colors::Black());
        ds.Close();
        blank_image = source;
        normal_brush = winrt::Microsoft::UI::Xaml::Media::SolidColorBrush(winrt::Microsoft::UI::Colors::Transparent());
        selected_brush = winrt::Microsoft::UI::Xaml::Media::SolidColorBrush(winrt::Microsoft::UI::Colors::LightSkyBlue());

        std::wostringstream os;
        os << L"Select a window to capture for \"" << target.c_str() << L"\"";
        title = std::move(hstring(os.str()));

        token_sizechanged = App::TopWindow().SizeChanged([this](auto const&, auto const& args){
            auto size = args.Size();
            reflect_top_window_size(size.Width, size.Height);
        });
        auto bounds = App::TopWindow().Bounds();
        reflect_top_window_size(bounds.Width, bounds.Height);
    }

    WindowPickerViewModel::~WindowPickerViewModel(){
        App::TopWindow().SizeChanged(token_sizechanged);
    }

    //============================================================================================
    // Start picker dialog
    //============================================================================================
    winrt::Windows::Foundation::IAsyncOperation<winrt::gui::WindowItem> WindowPickerViewModel::PickWindowAsync(){
        //------------------------------------------------------------------
        // List all available windows
        //------------------------------------------------------------------
        struct window_def{
            HWND hwnd;
            std::wstring title;
            long width;
            long height;
            window_def(HWND hwnd, const wchar_t* title, long width, long height) :
                hwnd(hwnd), title(title), width(width), height(height){}
        };
        std::vector<window_def> win_list;
        ::EnumWindows([](HWND hwnd, LPARAM context)->BOOL{
            auto is_visible = ::IsWindowVisible(hwnd);
            int is_cloaked;
            ::DwmGetWindowAttribute(hwnd, DWMWA_CLOAKED, &is_cloaked, sizeof(is_cloaked));
            if (is_visible && !is_cloaked){				
                auto& list = *reinterpret_cast<std::vector<window_def>*>(context);
                wchar_t title[1024];
                ::GetWindowTextW(hwnd, title, sizeof(title) - 1);
                RECT rect;
                ::GetWindowRect(hwnd, &rect);
                auto width = rect.right - rect.left;
                auto height = rect.bottom - rect.top;
                if (width > MINIMUM_ENABLE_WINDOW_SIZE && height > MINIMUM_ENABLE_WINDOW_SIZE &&
                    max(width, height) / min(width, height) < MAX_ENABLE_WINDOW_RATIO){
                    list.emplace_back(hwnd, title, rect.right - rect.left, rect.bottom - rect.top);
                }
            }
            return true;
        }, reinterpret_cast<LPARAM>(&win_list));

        //------------------------------------------------------------------
        // Build up window collection object
        //------------------------------------------------------------------
        for (auto& window : win_list){
            auto item = winrt::make<WindowItem>(
                *this, reinterpret_cast<uint64_t>(window.hwnd), std::move(hstring(window.title)));
            if (item.IsCapturable()){
                item.Image(blank_image);
                item.Background(normal_brush);
                item.StartCapture();
                window_items.Append(item);
            }
        }

        //------------------------------------------------------------------
        // Show dialog to specify a window
        //------------------------------------------------------------------
        dialog = winrt::make<winrt::gui::implementation::WindowPickerDialog>(*this);
        dialog.XamlRoot(App::TopWindow().Content().XamlRoot());
        co_await dialog.ShowAsync();
        dialog = nullptr;

        co_return selected_item;
    }

    //============================================================================================
    // Dialog event handling
    //============================================================================================
    void WindowPickerViewModel::ClickCaptureButton(
        winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&){
        dialog.Hide();
    }

    void WindowPickerViewModel::ClickCancelButton(
        winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&){
        selected_item = nullptr;
        update_property(L"SelectedItem");
        dialog.Hide();
    }

    //============================================================================================
    // Sellection handling
    //============================================================================================
    void WindowPickerViewModel::TrySelect(winrt::gui::WindowItem const& item){
        if (!selected_item){
            selected_item = item;
            item.Background(selected_brush);
        }if (selected_item.hWnd() != item.hWnd()){
            selected_item.Background(normal_brush);
            selected_item = item;
            item.Background(selected_brush);
        }
        update_property(L"IsSelected");
    }

    void WindowPickerViewModel::DecideSelection(winrt::gui::WindowItem const& item){
        selected_item = item;
        dialog.Hide();
    }

    //============================================================================================
    // Window image capturing
    //============================================================================================
    void WindowItem::prepare_capture(){
        auto namestr = name.c_str();
        const auto factory = get_activation_factory<winrt::Windows::Graphics::Capture::GraphicsCaptureItem>();
        const auto interop = factory.as<IGraphicsCaptureItemInterop>();
        try{
			interop->CreateForWindow(
				reinterpret_cast<HWND>(hwnd),
				winrt::guid_of<ABI::Windows::Graphics::Capture::IGraphicsCaptureItem>(),
				reinterpret_cast<void**>(winrt::put_abi(capturing.item)));
        }catch(...){
        }
    }

    void WindowItem::start_capture(){
        if (capturing.item){
            capturing.frame_pool = winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool::Create(
                capturing.canvas_device,
                winrt::Windows::Graphics::DirectX::DirectXPixelFormat::B8G8R8A8UIntNormalized,
                2, capturing.item.Size());
            
            winrt::gui::WindowItem self{*this};
            winrt::weak_ref<winrt::gui::WindowItem> weak_self = winrt::make_weak(self);
            capturing.token = capturing.frame_pool.FrameArrived([this, weak_self](auto const&, auto const&){
                if (weak_self.get()){
                    process_captured_frame(weak_self);
                }
            });
            capturing.session = capturing.frame_pool.CreateCaptureSession(capturing.item);
            capturing.session.IsCursorCaptureEnabled(false);
            capturing.session.StartCapture();
        }
    }

    void WindowItem::stop_capture(){
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

    winrt::Windows::Foundation::IAsyncAction WindowItem::process_captured_frame(winrt::weak_ref<winrt::gui::WindowItem> weak_self){
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
                        Image(source);
                    }
                }
            }
            stop_capture();
        }
    }
}
