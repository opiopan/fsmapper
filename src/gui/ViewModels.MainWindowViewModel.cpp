#include "pch.h"
#include "ViewModels.MainWindowViewModel.h"
#include "ViewModels.MainWindowViewModel.g.cpp"
#include "App.xaml.h"

#include <shobjidl.h>
#include <winrt/windows.storage.pickers.h>

namespace winrt::gui::ViewModels::implementation
{
    hstring MainWindowViewModel::ScriptName()
    {
        throw hresult_not_implemented();
    }

    winrt::Windows::Foundation::IAsyncAction MainWindowViewModel::ClickOpenButton(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args)
    {
        auto hwnd = winrt::gui::implementation::App::TopWindowHandle();
        auto picker = winrt::Windows::Storage::Pickers::FileOpenPicker();
        picker.as<IInitializeWithWindow>()->Initialize(hwnd);
        picker.FileTypeFilter().Append(L".lua");
        auto file = co_await picker.PickSingleFileAsync();
    }

    winrt::event_token MainWindowViewModel::PropertyChanged(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler)
    {
        throw hresult_not_implemented();
    }

    void MainWindowViewModel::PropertyChanged(winrt::event_token const& token) noexcept
    {
        throw hresult_not_implemented();
    }
}
