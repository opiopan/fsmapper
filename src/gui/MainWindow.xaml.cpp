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

#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

#include <cmath>
#include <chrono>

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
using namespace std::literals::chrono_literals;

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
            Activated([this](auto, WindowActivatedEventArgs args){
                is_active = args.WindowActivationState() != WindowActivationState::Deactivated;
                update_title_theme();
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

        ui_settings = winrt::Windows::UI::ViewManagement::UISettings();
        color_changed_token = ui_settings.ColorValuesChanged([this](auto&&, auto&&){
            this->DispatcherQueue().TryEnqueue([this]() {
                if (fsmapper::app_config.get_app_theme() == fsmapper::app_theme::system) {
                    update_title_bar_theme();
                }
            });
        });
        ApplyTheme();

        CheckAndInstallDCSExporter();
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
        fsmapper::app_config.load_cli_params();
        auto& cli_script_path = fsmapper::app_config.get_cli_script_path();
        if (cli_script_path){
            view_model.RestartScript(cli_script_path->operator std::wstring());
        }
        if (fsmapper::app_config.get_cli_launch_minimized()){
            return;
        }

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

    void MainWindow::update_title_bar_theme(){
        auto hwnd = get_hwnd();
        auto foreground = ui_settings.GetColorValue(winrt::Windows::UI::ViewManagement::UIColorType::Foreground);
        is_dark_mode = foreground.R > 128 && foreground.G > 128 && foreground.B > 128;
        switch (fsmapper::app_config.get_app_theme()) {
            case fsmapper::app_theme::light:
                is_dark_mode = false;
                break;
                case fsmapper::app_theme::dark:
                is_dark_mode = true;
                break;
            default:
                break;
        }
        auto value = is_dark_mode ? TRUE : FALSE;
        ::DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));
        auto window_id = winrt::GetWindowIdFromWindow(hwnd);
        auto app_window = winrt::Microsoft::UI::Windowing::AppWindow::GetFromWindowId(window_id);
        auto title_bar = app_window.TitleBar();
        auto res = winrt::Microsoft::UI::Xaml::Application::Current().Resources();
        auto get_color_from_res = [&](winrt::Windows::Foundation::IInspectable key){
            auto color = res.Lookup(key);
            return winrt::unbox_value<winrt::Windows::UI::Color>(color);
        };
        if (is_dark_mode){
            title_bar.ButtonForegroundColor(Windows::UI::Colors::White());
            title_bar.ButtonBackgroundColor(Windows::UI::Colors::Transparent());
            title_bar.ButtonHoverBackgroundColor(Windows::UI::ColorHelper::FromArgb(50, 255, 255, 255));
        }else{
            title_bar.ButtonForegroundColor(Windows::UI::Colors::Black());
            title_bar.ButtonBackgroundColor(Windows::UI::Colors::Transparent());
            title_bar.ButtonHoverBackgroundColor(Windows::UI::ColorHelper::FromArgb(50, 0, 0, 0));
        }
        update_title_theme();
    }

    void MainWindow::update_title_theme(){
        Windows::UI::Color color;
        if (is_dark_mode){
            if (is_active){
                color = Windows::UI::Colors::White();
            }else{
                color = Windows::UI::ColorHelper::FromArgb(100, 255, 255, 255);
            }
        }else{
            if (is_active){
                color = Windows::UI::Colors::Black();
            }else{
                color = Windows::UI::ColorHelper::FromArgb(90, 0, 0, 0);
            }
        }
        auto brush = Microsoft::UI::Xaml::Media::SolidColorBrush(color);
        AppTitle().Foreground(brush);
        MenuButton().Foreground(brush);
        AppIcon().Opacity(is_active ? 1.0 : 0.6);
    }

    void MainWindow::ApplyTheme(){
        auto theme = fsmapper::app_config.get_app_theme();
        auto requested_theme = theme == fsmapper::app_theme::light ? ElementTheme::Light :
                               theme == fsmapper::app_theme::dark ? ElementTheme::Dark :
                               ElementTheme::Default;
        this->Content().as<FrameworkElement>().RequestedTheme(requested_theme);
        update_title_bar_theme();
    }

    winrt::Windows::Foundation::IAsyncAction MainWindow::CheckAndInstallDCSExporter(){
        winrt::apartment_context ui_thread;
        co_await winrt::resume_after(500ms);

        auto mode = fsmapper::app_config.get_dcs_exporter_mode();
        if (mode != fsmapper::config::dcs_exporter_mode::off){
            dcs::installer installer;
            installer.check();
            co_await ui_thread;
            auto result = co_await dcs::confirm_change_export_lua(fsmapper::config::dcs_exporter_mode::unknown, installer);
            if (result == dcs::confirmation_yes){
                if (!installer.install()){
                    installer.show_install_error();
                    co_return;
                }
            }
            auto next = result == dcs::confirmation_unknown ? fsmapper::config::dcs_exporter_mode::unknown :
                        result == dcs::confirmation_yes ?     fsmapper::config::dcs_exporter_mode::on :
                        result == dcs::no_changes_needed ?    fsmapper::config::dcs_exporter_mode::on :
                                                              fsmapper::config::dcs_exporter_mode::off;
            fsmapper::app_config.set_dcs_exporter_mode(next);
        }
    }

}
