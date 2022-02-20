#pragma once
#include "ViewModels.MainWindowViewModel.g.h"

namespace winrt::gui::ViewModels::implementation
{
    struct MainWindowViewModel : MainWindowViewModelT<MainWindowViewModel>
    {
        MainWindowViewModel() = default;

        hstring ScriptName();
        winrt::Windows::Foundation::IAsyncAction ClickOpenButton(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args);
        winrt::event_token PropertyChanged(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler);
        void PropertyChanged(winrt::event_token const& token) noexcept;
    };
}
namespace winrt::gui::ViewModels::factory_implementation
{
    struct MainWindowViewModel : MainWindowViewModelT<MainWindowViewModel, implementation::MainWindowViewModel>
    {
    };
}
