//
// ViewModels.ConsolePageViewModel.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once
#include "ViewModels.ConsolePageViewModel.g.h"
#include "Models.h"

namespace winrt::gui::ViewModels::implementation{
    struct ConsolePageViewModel : ConsolePageViewModelT<ConsolePageViewModel>{
        ConsolePageViewModel();
        virtual ~ConsolePageViewModel();

        winrt::gui::Models::Mapper Mapper(){return mapper;}

        void EraseLog(winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&){
            mapper.Messages().Clear();
        }

        winrt::event_token PropertyChanged(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler){
            return property_changed.add(handler);
        }
        void PropertyChanged(winrt::event_token const& token) noexcept{
            property_changed.remove(token);
        }

    protected:
        winrt::gui::Models::Mapper mapper{nullptr};

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
namespace winrt::gui::ViewModels::factory_implementation{
    struct ConsolePageViewModel : ConsolePageViewModelT<ConsolePageViewModel, implementation::ConsolePageViewModel>{
    };
}
