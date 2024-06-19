#pragma once

#include "MainWindow.g.h"
#include "ViewModels.MainWindowViewModel.h"
#include <winrt/Microsoft.UI.Windowing.h>

using namespace winrt::Microsoft::UI::Xaml::Controls;

namespace winrt
{
    using namespace Microsoft::UI::Windowing;
    using namespace Microsoft::UI;
}

namespace winrt::gui::implementation
{
    struct MainWindow : MainWindowT<MainWindow>
    {
        MainWindow();
        winrt::gui::ViewModels::MainWindowViewModel ViewModel(){return view_model;}

        void NavView_Loaded(
            Windows::Foundation::IInspectable const&,
            Microsoft::UI::Xaml::RoutedEventArgs const&);
        void NavView_SelectionChanged(
            NavigationView const&,
            NavigationViewSelectionChangedEventArgs const& args);
        void MenuButton_Click(
            winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args);
        void GuideMenu_Click(
            winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args);
        void GithubMenu_Click(
            winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args);
        void ReleaseMenu_Click(
            winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args);

    private:
        static winrt::gui::ViewModels::MainWindowViewModel view_model;
        using page_data = std::pair<std::wstring, Windows::UI::Xaml::Interop::TypeName>;
        std::vector<page_data> pages;
        winrt::event_token closing_event_token;
        winrt::event_token activate_event_token;

        void set_region_for_title_bar();

        HWND get_hwnd() {
            HWND hwnd{ 0 };
            auto window_native{ this->try_as<::IWindowNative>() };
            window_native->get_WindowHandle(&hwnd);
            return hwnd;
        }

        winrt::AppWindow MainWindow::GetAppWindowForCurrentWindow();
        void save_window_position();
        void restore_window_position();

        void activate_window();
    };
}

namespace winrt::gui::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {
    };
}
