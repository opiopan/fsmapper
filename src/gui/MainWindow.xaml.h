#pragma once

#include "MainWindow.g.h"

using namespace winrt::Microsoft::UI::Xaml::Controls;

namespace winrt::gui::implementation
{
    struct MainWindow : MainWindowT<MainWindow>
    {
        MainWindow();
        void NavView_Loaded(
            Windows::Foundation::IInspectable const&,
            Microsoft::UI::Xaml::RoutedEventArgs const&);
        void NavView_SelectionChanged(
            NavigationView const&,
            NavigationViewSelectionChangedEventArgs const& args);

    private:
        using page_data = std::pair<std::wstring, Windows::UI::Xaml::Interop::TypeName>;
        std::vector<page_data> pages;

        HWND get_hwnd() {
            HWND hwnd{ 0 };
            auto window_native{ this->try_as<::IWindowNative>() };
            window_native->get_WindowHandle(&hwnd);
            return hwnd;
        }
        void save_window_position();
        void restore_window_position();
    };
}

namespace winrt::gui::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {
    };
}
