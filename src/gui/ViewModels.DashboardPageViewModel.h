#pragma once
#include "ViewModels.DashboardPageViewModel.g.h"
#include "Models.Mapper.h"

namespace winrt::gui::ViewModels::implementation
{
    struct DashboardPageViewModel : DashboardPageViewModelT<DashboardPageViewModel>
    {
        DashboardPageViewModel();

        bool SimIconIsVisible();
        hstring SimIconSource();
        hstring SimString();
        
        winrt::event_token PropertyChanged(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler);
        void PropertyChanged(winrt::event_token const& token) noexcept;

    protected:
        winrt::gui::Models::Mapper mapper{nullptr};
        winrt::event_token token_for_mapper;

        bool sim_icon_is_visible;
        hstring sim_icon_source;
        hstring sim_string;

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

    };
}
namespace winrt::gui::ViewModels::factory_implementation
{
    struct DashboardPageViewModel : DashboardPageViewModelT<DashboardPageViewModel, implementation::DashboardPageViewModel>
    {
    };
}
