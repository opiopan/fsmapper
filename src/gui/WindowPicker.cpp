//
// WindowPicker.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//
#include "pch.h"
#include "WindowPicker.h"
#include "Dialogs.WindowItem.g.cpp"
#include "Dialogs.WindowPickerViewModel.g.cpp"

namespace winrt::gui::Dialogs{
    //============================================================================================
    // Pincking entry function
    //============================================================================================
    winrt::Windows::Foundation::IAsyncOperation<winrt::gui::Dialogs::WindowItem> PickWindowAsync(){
        auto picker = winrt::make<winrt::gui::Dialogs::implementation::WindowPickerViewModel>();
        auto window = co_await picker.PickWindowAsync();
        co_return window;
    }
}

namespace winrt::gui::Dialogs::implementation{
    //============================================================================================
    // Start picker dialog
    //============================================================================================
    winrt::Windows::Foundation::IAsyncOperation<winrt::gui::Dialogs::WindowItem> WindowPickerViewModel::PickWindowAsync(){
        co_return selected_item;
    }
}