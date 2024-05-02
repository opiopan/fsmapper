//
// ViewModels.DashboardPageViewModel.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//
#pragma once
#include "ViewModels.DashboardPageViewModel.g.h"
#include "Models.h"

#include <winrt/Microsoft.UI.Xaml.Media.Imaging.h>
#include <string>

//============================================================================================
// View model definition
//============================================================================================
namespace winrt::gui::ViewModels::implementation{
    struct DashboardPageViewModel : DashboardPageViewModelT<DashboardPageViewModel>{
        DashboardPageViewModel();
        ~DashboardPageViewModel();

        winrt::gui::Models::Mapper Mapper(){return mapper;}
        bool NormalViewIsVisible(){return normal_view_is_visible;}
        bool StopViewIsVisible(){return stop_view_is_visible;}
        bool ErrorViewIsVisible(){return error_view_is_visible;}
        bool SimIconIsVisible(){return sim_icon_is_visible;}
        winrt::Microsoft::UI::Xaml::Media::Imaging::BitmapImage SimIconSource(){
            return logo_images[static_cast<int>(sim_type)];
        }
        hstring SimString(){return hstring(sim_name);}
        bool AircraftNameIsVisible(){return aircraft_name.length() >0;}
        hstring AircraftName(){return hstring(aircraft_name);}
        hstring DeviceSummary(){return device_summary;}
        hstring DeviceDetail(){return device_detail;}
        bool DeviceDetailIsVisible(){return device_detail_is_visible;}
        hstring MappingsSummary(){return mappings_summary;}
        int32_t MappingsDetailPrimery(){return mappings_detail_primery;}
        int32_t MappingsDetailSecondary(){return mappings_detail_secondary;}
        int32_t MappingsDetailViewports(){return mappings_detail_viewports;}
        int32_t MappingsDetailViews(){return mappings_detail_views;}
        bool MappingsDetailIsVisible(){return mappings_detail_is_visible;}
        hstring ViewportSummary(){return viewport_summary;}
        hstring ViewportDetail(){return viewport_detail;}
        bool ViewportDetailIsVisible(){return viewport_detail_is_visible;}
        bool ViewportButtonIsEnabled(){return viewport_button_is_enabled;}
        bool ViewportButtonIsVisible(){return viewport_button_is_visible;}
        hstring ViewportButtonText(){return viewport_button_text;}
        bool CapturedWindowsIsVisible(){return captured_windows_is_visible;}
        hstring CapturedWindowsSummary(){return captured_windows_summary;}
        bool CaptureWindowButtonIsEnabled(){return captured_window_button_is_enabled;}
        winrt::Microsoft::UI::Xaml::Style StartStopViewportsButtonStyle();

        winrt::Windows::Foundation::IAsyncAction ToggleViewport(
            winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args);

        winrt::event_token PropertyChanged(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler){
            return property_changed.add(handler);
        }
        void PropertyChanged(winrt::event_token const& token) noexcept{
            return property_changed.remove(token);
        }

    protected:
        winrt::gui::Models::Mapper mapper{nullptr};
        winrt::event_token token_for_mapper;
        winrt::event_token token_for_captured_windows;
        winrt::Microsoft::UI::Xaml::Media::Imaging::BitmapImage logo_images[3];

        bool normal_view_is_visible{false};
        bool stop_view_is_visible{true};
        bool error_view_is_visible{false};
        bool sim_icon_is_visible;
        winrt::gui::Models::Simulators sim_type;
        std::wstring sim_name;
        std::wstring aircraft_name;
        hstring device_summary;
        hstring device_detail;
        bool device_detail_is_visible{false};
        hstring mappings_summary;
        int32_t mappings_detail_primery{0};
        int32_t mappings_detail_secondary{0};
        int32_t mappings_detail_viewports{0};
        int32_t mappings_detail_views{0};
        bool mappings_detail_is_visible{false};
        hstring viewport_summary;
        hstring viewport_detail;
        bool viewport_detail_is_visible{false};
        bool viewport_button_is_enabled{false};
        bool viewport_button_is_visible{false};
        hstring viewport_button_text;
        bool captured_windows_is_visible{false};
        hstring captured_windows_summary;
        bool captured_window_button_is_enabled{false};
        bool all_captured_window_is_captured{false};

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

        void reflect_mapper_Status();
        void reflect_mapper_ActiveSim();
        void reflect_mapper_AircraftName();
        void reflect_mapper_MappingsInfo();
        void reflect_mapper_Viewports();
        void reflect_mapper_Devices();
        void reflect_mapper_CapturedWindows();
        void reflect_mapper_ViewportOperability();
    };
}
namespace winrt::gui::ViewModels::factory_implementation{
    struct DashboardPageViewModel : DashboardPageViewModelT<DashboardPageViewModel, implementation::DashboardPageViewModel>{
    };
}

//============================================================================================
// Converters
//============================================================================================
