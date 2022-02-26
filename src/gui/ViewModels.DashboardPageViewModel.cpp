#include "pch.h"
#include "ViewModels.DashboardPageViewModel.h"
#include "ViewModels.DashboardPageViewModel.g.cpp"

#include "App.xaml.h"

using namespace winrt;
using namespace winrt::Microsoft::UI::Xaml;
using App = winrt::gui::implementation::App;

namespace winrt::gui::ViewModels::implementation
{
    //============================================================================================
    // Object constructor / destructor
    //============================================================================================
    DashboardPageViewModel::DashboardPageViewModel(){
        mapper = App::Mapper();
    }

    //============================================================================================
    // Properties of runtime class
    //============================================================================================
    bool DashboardPageViewModel::SimIconIsVisible(){
        return sim_icon_is_visible;
    }

    hstring DashboardPageViewModel::SimIconSource(){
        return sim_icon_source;
    }

    hstring DashboardPageViewModel::SimString(){
        return sim_string;
    }

    //============================================================================================
    // Event notification
    //============================================================================================
    winrt::event_token DashboardPageViewModel::PropertyChanged(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler){
        return property_changed.add(handler);
    }

    void DashboardPageViewModel::PropertyChanged(winrt::event_token const& token) noexcept{
        return property_changed.remove(token);
    }
}
