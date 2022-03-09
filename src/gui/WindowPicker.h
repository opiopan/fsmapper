//
// WindowPicker.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once
#include "Dialogs.WindowItem.g.h"
#include "Dialogs.WindowPickerViewModel.g.h"

//============================================================================================
// WindowItem
//============================================================================================
namespace winrt::gui::Dialogs::implementation{
    struct WindowItem : WindowItemT<WindowItem>{
        WindowItem() = delete;
        WindowItem(uint64_t hwnd, hstring const& name): hwnd(hwnd), name(name){}

        uint64_t hWnd(){return hwnd;}
        hstring Name(){return name;}
        winrt::Microsoft::UI::Xaml::Media::ImageSource Image(){return image;}
        void Image(winrt::Microsoft::UI::Xaml::Media::ImageSource const& value){
            image = value;
            property_changed(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs{L"Image"});
        }

        winrt::event_token PropertyChanged(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler){
            return property_changed.add(handler);
        }
        void PropertyChanged(winrt::event_token const& token) noexcept{
            property_changed.remove(token);
        }

    protected:
        uint64_t hwnd{0};
        hstring name{0};
        winrt::Microsoft::UI::Xaml::Media::ImageSource image{nullptr};

        winrt::event<Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> property_changed;
    };
}
namespace winrt::gui::Dialogs::factory_implementation{
    struct WindowItem : WindowItemT<WindowItem, implementation::WindowItem>{
    };
}

//============================================================================================
// Pincking function
//============================================================================================
namespace winrt::gui::Dialogs{
    winrt::Windows::Foundation::IAsyncOperation<winrt::gui::Dialogs::WindowItem> PickWindowAsync();
}

//============================================================================================
// WindowPickerViewModel
//============================================================================================
namespace winrt::gui::Dialogs::implementation{
    using WindowItemCollection = winrt::Windows::Foundation::Collections::IObservableVector<winrt::gui::Dialogs::WindowItem>;

    struct WindowPickerViewModel : WindowPickerViewModelT<WindowPickerViewModel>{
        WindowPickerViewModel(){
            window_items = winrt::single_threaded_observable_vector<gui::Dialogs::WindowItem>();
        }

        winrt::gui::Dialogs::WindowItem SelectedItem(){return selected_item;}
        WindowItemCollection WindowItems(){return window_items;}
        winrt::Windows::Foundation::IAsyncOperation<winrt::gui::Dialogs::WindowItem> PickWindowAsync();

        winrt::event_token PropertyChanged(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler){
            return property_changed.add(handler);
        }
        void PropertyChanged(winrt::event_token const& token) noexcept{
            property_changed.remove(token);
        }

    protected:
        WindowItemCollection window_items{nullptr};
        winrt::gui::Dialogs::WindowItem selected_item{nullptr};

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
    };
}
namespace winrt::gui::Dialogs::factory_implementation{
    struct WindowPickerViewModel : WindowPickerViewModelT<WindowPickerViewModel, implementation::WindowPickerViewModel>{
    };
}
