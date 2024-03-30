#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif
#include <winrt/Windows.UI.Xaml.Interop.h>
#include <winrt/Microsoft.UI.Windowing.h>
#include <winrt/Microsoft.UI.Interop.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>

#include "config.hpp"
#include "DashboardPage.xaml.h"
#include "ConsolePage.xaml.h"
#include "UtilitiesPage.xaml.h"
#include "SettingsPage.xaml.h"
#include "App.xaml.h"
#include "resource.h"
#include "../.version.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;

using App = winrt::gui::implementation::App;

namespace winrt::gui::implementation
{
    winrt::gui::ViewModels::MainWindowViewModel MainWindow::view_model{nullptr};

    MainWindow::MainWindow(){
        InitializeComponent();

        this->ExtendsContentIntoTitleBar(true);
        this->SetTitleBar(AppTitleBar());
        auto appname = Application::Current().Resources().Lookup(winrt::box_value(L"AppName"));
        this->Title(unbox_value<winrt::hstring>(appname));

        AppVersion().Text(L"v" VERSTR_FILE_VERSION);

        auto module = ::GetModuleHandleW(nullptr);
        auto icon = ::LoadIconW(module, MAKEINTRESOURCEW(IDI_APP_ICON));
        HWND hwnd{ nullptr };
        this->try_as<IWindowNative>()->get_WindowHandle(&hwnd);
        ::SendMessageW(hwnd, WM_SETICON, 1, reinterpret_cast<LPARAM>(icon));

        view_model = winrt::make<winrt::gui::ViewModels::implementation::MainWindowViewModel>();

        pages.emplace_back(page_data(L"dashboard", xaml_typename<gui::DashboardPage>()));
        pages.emplace_back(page_data(L"console", xaml_typename<gui::ConsolePage>()));
        pages.emplace_back(page_data(L"utilities", xaml_typename<gui::UtilitiesPage>()));
        pages.emplace_back(page_data(L"settings", xaml_typename<gui::SettingsPage>()));

        restore_window_position();

        auto app_window = GetAppWindowForCurrentWindow();
        closing_event_token = app_window.Closing([this](const auto&, const auto&) {
            App::Mapper().StopScriptSync();
            save_window_position();
        });
    }

    void MainWindow::NavView_Loaded(
        Windows::Foundation::IInspectable const&,
        Microsoft::UI::Xaml::RoutedEventArgs const&){
        NavView().SelectedItem(NavView().MenuItems().GetAt(0));
    }

    void MainWindow::NavView_SelectionChanged(
        NavigationView const&,
        NavigationViewSelectionChangedEventArgs const& args){
        if (args.SelectedItemContainer()) {
            auto tag = winrt::unbox_value_or<winrt::hstring>(
                args.SelectedItemContainer().Tag(), L"");
            for (auto&& page : pages) {
                if (page.first == tag) {
                    ContentFrame().Navigate(
                        page.second, nullptr, args.RecommendedNavigationTransitionInfo());
                }
            }
        }
    }

    winrt::AppWindow MainWindow::GetAppWindowForCurrentWindow() {
        HWND hwnd{ nullptr };
        this->try_as<IWindowNative>()->get_WindowHandle(&hwnd);
        auto winid = winrt::GetWindowIdFromWindow(hwnd);
        auto app_window = winrt::Microsoft::UI::Windowing::AppWindow::GetFromWindowId(winid);
        return app_window;
    }

    void MainWindow::save_window_position(){
        HWND hwnd{ nullptr };
        this->try_as<IWindowNative>()->get_WindowHandle(&hwnd);
        RECT rect;
        ::GetWindowRect(hwnd, &rect);
        fsmapper::rect crect{ rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top };
        fsmapper::app_config.set_window_rect(crect);
        fsmapper::app_config.save();
    }

    void MainWindow::restore_window_position(){
        auto& rect = fsmapper::app_config.get_window_rect();
        if (rect.width <= 0 || rect.height <= 0) {
            return;
        }
        HWND hwnd{ nullptr };
        this->try_as<IWindowNative>()->get_WindowHandle(&hwnd);
        ::MoveWindow(hwnd, rect.left, rect.top, rect.width, rect.height, true);
    }
}
