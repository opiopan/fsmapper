#include "pch.h"

#include "App.xaml.h"
#include "MainWindow.xaml.h"
#include "config.hpp"
#include  "cli_params.hpp"
#include  "../.version.h"

#include <winrt/Windows.UI.Xaml.Interop.h>
#include <winrt/Microsoft.UI.Windowing.h>
#include <winrt/Microsoft.UI.Interop.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.Windows.AppLifecycle.h>

#include <filesystem>

using namespace winrt;
using namespace Windows::Foundation;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Microsoft::UI::Xaml::Navigation;
using namespace Microsoft::Windows::AppLifecycle;
using namespace gui;
using namespace gui::implementation;

static void cleanup_older_version(){
    auto&& module_dir = fsmapper::app_config.get_module_dir();
    std::error_code ec;
    for (const auto& entry : std::filesystem::directory_iterator(module_dir.parent_path(), ec)){
        if (ec){
            break;
        }
        if (entry.is_regular_file()){
            auto&& filename = entry.path().filename().string();
            if ((filename.rfind("fsmapperhook_") == 0 || filename == "fsmapperhook.dll") &&
                filename.rfind("fsmapperhook_" VERSTR_TITLE_VERSION) != 0){
                std::error_code ec2;
                std::filesystem::remove(entry.path(), ec2);
            }
        }
    }
}

winrt::gui::Models::Mapper App::mapper{nullptr};
winrt::Microsoft::UI::Xaml::Window App::window{nullptr};

App::App(){
    fsmapper::init_app_config();
    fsmapper::cli_params params;
    fsmapper::app_config.set_cli_launch_minimized(params.launch_minimized);
    fsmapper::app_config.set_cli_script_path(params.script_path);
    if (params.script_path){
        std::filesystem::path path{params.script_path};
        fsmapper::app_config.set_script_path(std::move(path));
    }

#if defined _DEBUG && !defined DISABLE_XAML_GENERATED_BREAK_ON_UNHANDLED_EXCEPTION
    UnhandledException([this](IInspectable const&, UnhandledExceptionEventArgs const& e)
    {
        if (IsDebuggerPresent())
        {
            auto errorMessage = e.Message();
            __debugbreak();
        }
    });
#endif
}

HWND App::TopWindowHandle() {
    HWND hwnd{ nullptr };
    TopWindow().try_as<IWindowNative>()->get_WindowHandle(&hwnd);
    return hwnd;
}

winrt::fire_and_forget App::OnLaunched(LaunchActivatedEventArgs const&){
    auto mainInstance{ AppInstance::FindOrRegisterForKey(L"fsmapper") };
    if (!mainInstance.IsCurrent()){
        // Redirect the activation (and args) to the "main" instance, and exit.
        fsmapper::app_config.save_cli_params();
        auto activatedEventArgs{AppInstance::GetCurrent().GetActivatedEventArgs()};
        co_await mainInstance.RedirectActivationToAsync(activatedEventArgs);
        ::ExitProcess(0);
        co_return;
    }

    cleanup_older_version();

    mapper = winrt::make<gui::Models::implementation::Mapper>();
    window = make<MainWindow>();
    window.Activate();

    // Minimize window if /i option is specified
    if (fsmapper::app_config.get_cli_launch_minimized()){
        auto hwnd = TopWindowHandle();
        ::ShowWindow(hwnd, SW_MINIMIZE);
    }
}