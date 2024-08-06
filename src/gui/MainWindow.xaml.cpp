#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif
#include <winrt/Windows.UI.Xaml.Interop.h>
#include <winrt/Microsoft.UI.Windowing.h>
#include <winrt/Microsoft.UI.Interop.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Composition.SystemBackdrops.h>
#include <winrt/Microsoft.UI.Input.h>
#include <winrt/Microsoft.Windows.AppLifecycle.h>

#include <cmath>

#include "config.hpp"
#include "DashboardPage.xaml.h"
#include "ConsolePage.xaml.h"
#include "UtilitiesPage.xaml.h"
#include "SettingsPage.xaml.h"
#include "App.xaml.h"
#include "resource.h"
#include "../.version.h"
#include "dcs_installer.hpp"

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::Windows::AppLifecycle;

using App = winrt::gui::implementation::App;

namespace winrt::gui::implementation
{
    winrt::gui::ViewModels::MainWindowViewModel MainWindow::view_model{nullptr};

    MainWindow::MainWindow(){
        InitializeComponent();

        if (winrt::Microsoft::UI::Composition::SystemBackdrops::MicaController().IsSupported()) {
            this->SystemBackdrop(Media::MicaBackdrop());
        }
        else if (winrt::Microsoft::UI::Composition::SystemBackdrops::DesktopAcrylicController().IsSupported()) {
            this->SystemBackdrop(Media::DesktopAcrylicBackdrop());
        }

        if (winrt::Microsoft::UI::Windowing::AppWindowTitleBar::IsCustomizationSupported()) {
            auto title_bar = AppWindow().TitleBar();
            title_bar.ExtendsContentIntoTitleBar(true);
            title_bar.ButtonBackgroundColor(Colors::Transparent());
            title_bar.ButtonInactiveBackgroundColor(Colors::Transparent());
            title_bar.PreferredHeightOption(TitleBarHeightOption::Tall);
            AppTitleBar().Loaded([this](auto, auto){set_region_for_title_bar();});
            AppTitleBar().SizeChanged([this](auto, auto){set_region_for_title_bar();});
            Activated([this](auto sender, WindowActivatedEventArgs args){
                if (args.WindowActivationState() == WindowActivationState::Deactivated) {
                    auto brush = unbox_value<Media::SolidColorBrush>(tools::AppResource(L"WindowCaptionForegroundDisabled"));
                    AppTitle().Foreground(brush);
                    AppVersion().Foreground(brush);
                }else{
                    auto brush = unbox_value<Media::SolidColorBrush>(tools::AppResource(L"WindowCaptionForeground"));
                    AppTitle().Foreground(brush);
                    AppVersion().Foreground(brush);
                }
            });
        }

        auto appname = unbox_value<winrt::hstring>(tools::AppResource(L"AppName"));
        this->Title(appname);

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

        auto this_instance = AppInstance::GetCurrent();
        activate_event_token = this_instance.Activated([this](const auto&, const AppActivationArguments&){
            activate_window();
        });

        dcs::installer installer;
        if (installer.check()){
            installer.install();
        }
    }

    void MainWindow::set_region_for_title_bar(){
        auto scale = AppTitleBar().XamlRoot().RasterizationScale();

        RightPaddingColumn().Width(GridLength{AppWindow().TitleBar().RightInset() / scale});
        LeftPaddingColumn().Width(GridLength{AppWindow().TitleBar().LeftInset() / scale});

        auto rect_of_control = [scale](auto control) {
            auto transform = control.TransformToVisual(nullptr);
            auto bounds = transform.TransformBounds({0, 0, static_cast<float>(control.ActualWidth()), static_cast<float>(control.ActualHeight())});
            return winrt::Windows::Graphics::RectInt32{
                static_cast<int32_t>(std::round(bounds.X * scale)),
                static_cast<int32_t>(std::round(bounds.Y * scale)),
                static_cast<int32_t>(std::round(bounds.Width * scale)),
                static_cast<int32_t>(std::round(bounds.Height * scale)),
            };
        };

        std::vector<winrt::Windows::Graphics::RectInt32> regions{
            rect_of_control(MenuButton()),
            rect_of_control(OpenButton()),
            rect_of_control(StartStopButton()),
            rect_of_control(HelpButton()),
        };
        auto non_client_input_source = winrt::Microsoft::UI::Input::InputNonClientPointerSource::GetForWindowId(AppWindow().Id());
        non_client_input_source.SetRegionRects(winrt::Microsoft::UI::Input::NonClientRegionKind::Passthrough, regions);
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

    void MainWindow::MenuButton_Click(
        winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&) {
        NavView().IsPaneOpen(!NavView().IsPaneOpen());
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
        WINDOWPLACEMENT wp;
        ::GetWindowPlacement(hwnd, &wp);
        auto& rect = wp.rcNormalPosition;
        auto scale = ::GetDpiForWindow(hwnd) / 96.0;
        fsmapper::rect crect{
            rect.left, rect.top, 
            static_cast<LONG>((rect.right - rect.left) / scale), static_cast<LONG>((rect.bottom - rect.top) / scale)
        };
        fsmapper::app_config.set_window_rect(crect);
        fsmapper::app_config.save();
    }

    void MainWindow::restore_window_position(){
        static constexpr auto minimum_height = 48;

        auto& rect = fsmapper::app_config.get_window_rect();
        if (rect.width <= 0 || rect.height <= minimum_height) {
            return;
        }
        HWND hwnd{ nullptr };
        this->try_as<IWindowNative>()->get_WindowHandle(&hwnd);
        auto scale = ::GetDpiForWindow(hwnd) / 96.0;
        ::MoveWindow(hwnd, rect.left, rect.top, static_cast<int>(rect.width * scale), static_cast<int>(rect.height * scale), true);
    }

    void MainWindow::activate_window(){
        auto hwnd{get_hwnd()};
        if (!hwnd){
            return;
        }

        auto current_wnd{GetForegroundWindow()};
		DWORD pid{ 0 };
        auto this_tid{GetCurrentThreadId()};
        auto current_tid{GetWindowThreadProcessId(current_wnd, &pid)};
        if (this_tid != current_tid){
            AllowSetForegroundWindow(static_cast<DWORD>(-1));
        }
        if (IsIconic(hwnd)){
            ShowWindow(hwnd, SW_RESTORE);
        }
        auto hwnd_last_active_popup{GetLastActivePopup(hwnd)};
        SwitchToThisWindow(hwnd_last_active_popup, true);
        SetForegroundWindow(hwnd);
        if (this_tid != current_tid){
            AttachThreadInput(this_tid, current_tid, false);
        }
    }
}
