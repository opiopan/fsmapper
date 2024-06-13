//
// ViewModels.MainWindowViewModel.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//
#pragma once
#include "ViewModels.MainWindowViewModel.g.h"
#include "Models.h"

namespace winrt::gui::ViewModels::implementation
{
    struct MainWindowViewModel : MainWindowViewModelT<MainWindowViewModel>
    {
        MainWindowViewModel();
        virtual ~MainWindowViewModel();

        hstring ScriptName();
        winrt::Microsoft::UI::Xaml::Media::SolidColorBrush ScriptStatusBrush();
        bool OpenButtonIsEnabled();
        bool StartStopButtonIsEnabled();
        hstring StartStopButtonIcon();
        hstring StartStopButtonLabel();
        hstring StartStopButtonToolTip();
        bool NewReleaseNotificationIsNecessary();
        hstring LatestVersion();

        winrt::Windows::Foundation::IAsyncAction ClickOpenButton(
            winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args);
        void ClickStartStopButton(
            winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args);
        void MenuButton_Click(
            winrt::Windows::Foundation::IInspectable const &sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const &args);
        void GuideMenu_Click(
            winrt::Windows::Foundation::IInspectable const &sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const &args);
        void GithubMenu_Click(
            winrt::Windows::Foundation::IInspectable const &sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const &args);
        void ReleaseMenu_Click(
            winrt::Windows::Foundation::IInspectable const &sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const &args);
        void ClickCloseNewReleaseButton(
            winrt::Windows::Foundation::IInspectable const &sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const &args);
        void ClickDownloadNewReleaseButton(
            winrt::Windows::Foundation::IInspectable const &sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const &args);

        winrt::event_token PropertyChanged(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler);
        void PropertyChanged(winrt::event_token const& token) noexcept;

    protected:
        winrt::gui::Models::Mapper mapper{nullptr};
        winrt::event_token token_for_mapper;
        winrt::Microsoft::UI::Xaml::Media::SolidColorBrush status_brushes[4];
        winrt::Windows::Foundation::IAsyncAction error_flasher{nullptr};
        int error_flash_thread_counter{0};

        winrt::hstring script_name;
        enum class StatusBrush {stop = 0, running, error_light, error_dark};
        StatusBrush script_status_brush{StatusBrush::stop};
        bool open_button_is_enabled{false};
        bool start_stop_button_is_enabled{false};
        hstring start_stop_button_icon;
        hstring start_stop_button_label;
        hstring start_stop_button_tool_tip;
        bool is_ignored_new_release_notification{false};
        bool new_release_is_available{false};

        winrt::event<Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> property_changed;

        template <typename T>
        void update_property(T& variable, const T& value, const wchar_t* name){
            if (variable != value){
                variable = value;
                property_changed(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs{name});
            }
        }

        template <typename T>
        void update_property(T& variable, T&& value, const wchar_t* name){
            if (variable != value){
                variable = std::move(value);
                property_changed(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs{name});
            }
        }

        void update_property(const wchar_t* name){
            property_changed(*this, Microsoft::UI::Xaml::Data::PropertyChangedEventArgs{name});
        }

        void reflect_mapper_ScriptPath();
        void reflect_mapper_Status();
        void reflect_mapper_NewRelease();
    };
}
namespace winrt::gui::ViewModels::factory_implementation
{
    struct MainWindowViewModel : MainWindowViewModelT<MainWindowViewModel, implementation::MainWindowViewModel>
    {
    };
}
