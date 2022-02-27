//
// ViewModels.DashboardPageViewModel.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//
#pragma once
#include "ViewModels.DashboardPageViewModel.g.h"
#include "Models.Mapper.h"

#include <winrt/Microsoft.UI.Xaml.Media.Imaging.h>
#include <string>

namespace winrt::gui::ViewModels::implementation{
    struct DashboardPageViewModel : DashboardPageViewModelT<DashboardPageViewModel>{
        DashboardPageViewModel();
        ~DashboardPageViewModel();

        winrt::gui::Models::Mapper Mapper();
        bool SimIconIsVisible();
        winrt::Microsoft::UI::Xaml::Media::Imaging::BitmapImage SimIconSource();
        hstring SimString();
        hstring DeviceSummary();
        hstring DeviceDetail();

        winrt::event_token PropertyChanged(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler){
            return property_changed.add(handler);
        }
        void PropertyChanged(winrt::event_token const& token) noexcept{
            return property_changed.remove(token);
        }

    protected:
        winrt::gui::Models::Mapper mapper{nullptr};
        winrt::event_token token_for_mapper;
        winrt::event_token token_for_devices;
        winrt::Microsoft::UI::Xaml::Media::Imaging::BitmapImage logo_images[3];

        bool sim_icon_is_visible;
        winrt::gui::Models::Simulators sim_type;
        std::wstring sim_name;
        std::wstring aircraft_name;
        hstring device_summary;
        hstring device_detail;

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

        void reflect_mapper_ActiveSim();
        void reflect_mapper_AircraftName();
        void reflect_devices();
    };
}
namespace winrt::gui::ViewModels::factory_implementation{
    struct DashboardPageViewModel : DashboardPageViewModelT<DashboardPageViewModel, implementation::DashboardPageViewModel>{
    };
}
