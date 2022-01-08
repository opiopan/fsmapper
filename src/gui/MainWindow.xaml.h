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
    };
}

namespace winrt::gui::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {
    };
}
