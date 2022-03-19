//
// WindowPickerDialog.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//
#pragma once

#include "WindowItem.g.h"
#include "WindowPickerViewModel.g.h"
#include "WindowPickerDialog.g.h"

#include <winrt/Microsoft.Graphics.Canvas.h>
#include <winrt/Windows.Graphics.Capture.h>

#include <sstream>

//============================================================================================
// WindowItem
//============================================================================================
namespace winrt::gui::implementation{
    struct WindowItem : WindowItemT<WindowItem>{
        WindowItem() = delete;
        WindowItem(winrt::gui::WindowPickerViewModel const& model, uint64_t hwnd, hstring const& name) :
            view_model(winrt::make_weak(model)), hwnd(hwnd), name(name){
            capturing.canvas_device = winrt::Microsoft::Graphics::Canvas::CanvasDevice::GetSharedDevice();
            prepare_capture();
        }
        ~WindowItem(){stop_capture();}

        uint64_t hWnd(){return hwnd;}
        hstring Name(){
            std::wostringstream os;
            os << std::hex << hwnd << " : " << name.c_str();
            return hstring(os.str());
        }
        bool IsCapturable(){
            return static_cast<bool>(capturing.item);
        }
        winrt::Microsoft::UI::Xaml::Media::ImageSource Image(){return image;}
        void Image(winrt::Microsoft::UI::Xaml::Media::ImageSource const& value){
            image = value;
            property_changed(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs{L"Image"});
        }
        winrt::Microsoft::UI::Xaml::Media::SolidColorBrush Background(){return background;}
        void Background(winrt::Microsoft::UI::Xaml::Media::SolidColorBrush const& value){
            background = value;
            property_changed(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs{L"Background"});
        }

        void TryTap(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::Input::TappedRoutedEventArgs const&){
            auto strong_ref = view_model.get();
            if (strong_ref){
                strong_ref.TrySelect(*this);
            }
        }

        void TryDoubleTap(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::Input::DoubleTappedRoutedEventArgs const&){
            auto strong_ref = view_model.get();
            if (strong_ref){
                strong_ref.DecideSelection(*this);
            }
        }

        void StartCapture(){start_capture();}

        winrt::event_token PropertyChanged(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler){
            return property_changed.add(handler);
        }
        void PropertyChanged(winrt::event_token const& token) noexcept{
            property_changed.remove(token);
        }

    protected:
        winrt::weak_ref<winrt::gui::WindowPickerViewModel> view_model;
        uint64_t hwnd{0};
        hstring name;
        winrt::Microsoft::UI::Xaml::Media::ImageSource image{nullptr};
        winrt::Microsoft::UI::Xaml::Media::SolidColorBrush background{nullptr};

        winrt::event<Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> property_changed;

        //------------------------------------------------------------------
        // Window image capturing context & functions
        //------------------------------------------------------------------
        struct {
            winrt::Microsoft::Graphics::Canvas::CanvasDevice canvas_device{nullptr};
            winrt::Windows::Graphics::Capture::GraphicsCaptureItem item{nullptr};
            winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool frame_pool{nullptr};
            winrt::Windows::Graphics::Capture::GraphicsCaptureSession session{nullptr};
            winrt::event_token token;
        }capturing;

        void prepare_capture();
        void start_capture();
        void stop_capture();

        winrt::Windows::Foundation::IAsyncAction process_captured_frame(winrt::weak_ref<winrt::gui::WindowItem> weak_self);
    };
}
namespace winrt::gui::factory_implementation{
    struct WindowItem : WindowItemT<WindowItem, implementation::WindowItem>{
    };
}

//============================================================================================
// WindowPickerViewModel
//============================================================================================
namespace winrt::gui::implementation{
    using WindowItemCollection = winrt::Windows::Foundation::Collections::IObservableVector<winrt::gui::WindowItem>;

    struct WindowPickerViewModel : WindowPickerViewModelT<WindowPickerViewModel>{
        WindowPickerViewModel() = delete;
        WindowPickerViewModel(hstring const& target);
        ~WindowPickerViewModel();

        winrt::gui::WindowItem SelectedItem(){return selected_item;}
        WindowItemCollection WindowItems(){return window_items;}
        hstring Title(){return title;}
        winrt::Windows::Foundation::IAsyncOperation<winrt::gui::WindowItem> PickWindowAsync();
        bool IsSelected(){return selected_item != nullptr;}

        float BoundsWidth(){return bounds_width;}
        float BoundsHeight(){return bounds_height;}

        void ClickCaptureButton(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args);
        void ClickCancelButton(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args);
        void TrySelect(winrt::gui::WindowItem const& item);
        void DecideSelection(winrt::gui::WindowItem const& item);

        winrt::event_token PropertyChanged(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler){
            return property_changed.add(handler);
        }
        void PropertyChanged(winrt::event_token const& token) noexcept{
            property_changed.remove(token);
        }

    protected:
        WindowItemCollection window_items{nullptr};
        winrt::gui::WindowItem selected_item{nullptr};
        hstring title;
        float bounds_width{0.f};
        float bounds_height{0.f};

        winrt::Microsoft::UI::Xaml::Media::ImageSource blank_image{nullptr};
        winrt::event_token token_sizechanged;
        winrt::gui::WindowPickerDialog dialog{nullptr};
        winrt::Microsoft::UI::Xaml::Media::SolidColorBrush normal_brush{nullptr};
        winrt::Microsoft::UI::Xaml::Media::SolidColorBrush selected_brush{nullptr};

        winrt::event<Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> property_changed;
        template <typename T>
        void update_property(T& variable, const T& value, const wchar_t* name){
            if (variable != value){
                variable = value;
                property_changed(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs{name});
            }
        }

        template <typename T>
        void update_property(T& variable, T&& value, const wchar_t* name){
            if (variable != value){
                variable = std::move(value);
                property_changed(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs{name});
            }
        }

        void update_property(const wchar_t* name){
            property_changed(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs{name});
        }

        void reflect_top_window_size(float width, float height){
            update_property(bounds_width, min(width - 150.f, 1200.f), L"BoundsWidth");
            update_property(bounds_height, min(height - 150.f, 900.f), L"BoundsHeight");
        }
    };
}
namespace winrt::gui::factory_implementation{
    struct WindowPickerViewModel : WindowPickerViewModelT<WindowPickerViewModel, implementation::WindowPickerViewModel>{
    };
}

//============================================================================================
// WindowPickerDialog
//============================================================================================
namespace winrt::gui::implementation{
    struct WindowPickerDialog : WindowPickerDialogT<WindowPickerDialog>{
        WindowPickerDialog() = delete;
        WindowPickerDialog(winrt::gui::WindowPickerViewModel const& view_model) : view_model(winrt::make_weak(view_model)){
            InitializeComponent();
        }
        winrt::gui::WindowPickerViewModel ViewModel(){return view_model.get();}

    protected:
        winrt::weak_ref<winrt::gui::WindowPickerViewModel> view_model;
    };
}
namespace winrt::gui::factory_implementation{
    struct WindowPickerDialog : WindowPickerDialogT<WindowPickerDialog, implementation::WindowPickerDialog>{
    };
}

//============================================================================================
// Pincking function
//============================================================================================
namespace winrt::gui{
    winrt::Windows::Foundation::IAsyncOperation<winrt::gui::WindowItem> PickWindowAsync(hstring const& target);
}
