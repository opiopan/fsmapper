#include "pch.h"

#include "App.xaml.h"
#include "MainWindow.xaml.h"
#include "config.hpp"

#include <winrt/Windows.UI.Xaml.Interop.h>
#include <winrt/Microsoft.UI.Windowing.h>
#include <winrt/Microsoft.UI.Interop.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>

using namespace winrt;
using namespace Windows::Foundation;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Microsoft::UI::Xaml::Navigation;
using namespace gui;
using namespace gui::implementation;

winrt::gui::Models::Mapper App::mapper{nullptr};
winrt::Microsoft::UI::Xaml::Window App::window{nullptr};

App::App()
{
    fsmapper::init_app_config();

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

void App::OnLaunched(LaunchActivatedEventArgs const&)
{
    mapper = winrt::make<gui::Models::implementation::Mapper>();
    window = make<MainWindow>();
    window.Activate();
}