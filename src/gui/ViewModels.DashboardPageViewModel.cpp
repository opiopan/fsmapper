//
// ViewModels.DashboardPageViewModel.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//
#include "pch.h"
#include "ViewModels.DashboardPageViewModel.h"
#include "ViewModels.DashboardPageViewModel.g.cpp"

#include "App.xaml.h"
#include "tools.hpp"

#include <string>

using namespace winrt;
using namespace winrt::Microsoft::UI::Xaml;
using App = winrt::gui::implementation::App;

namespace winrt::gui::ViewModels::implementation{
    //============================================================================================
    // Object constructor / destructor
    //============================================================================================
    DashboardPageViewModel::DashboardPageViewModel(){
        using namespace winrt::gui::Models;
        logo_images[static_cast<int>(Simulators::none)] = nullptr;
        logo_images[static_cast<int>(Simulators::fs2020)] = 
            unbox_value<Media::Imaging::BitmapImage>(tools::AppResource(L"SimLogoFS2020"));
        logo_images[static_cast<int>(Simulators::dcs)] = 
            unbox_value<Media::Imaging::BitmapImage>(tools::AppResource(L"SimLogoDCS"));

        mapper = App::Mapper();
        token_for_mapper = mapper.PropertyChanged([this](auto const&, auto const& args) {
            auto name = args.PropertyName();
            if (name == L"ActiveSim"){
                reflect_mapper_ActiveSim();
            }else if (name == L"AircraftName"){
                reflect_mapper_AircraftName();
            }
        });
        reflect_mapper_ActiveSim();
        reflect_mapper_AircraftName();
    }

    DashboardPageViewModel::~DashboardPageViewModel(){
        mapper.PropertyChanged(token_for_mapper);
    }

    //============================================================================================
    // Properties of runtime class
    //============================================================================================
    bool DashboardPageViewModel::SimIconIsVisible(){
        return sim_icon_is_visible;
    }

    Media::Imaging::BitmapImage DashboardPageViewModel::SimIconSource(){
        return logo_images[static_cast<int>(sim_type)];
    }

    hstring DashboardPageViewModel::SimString(){
        return hstring(sim_name + aircraft_name);
    }

    //============================================================================================
    // Reflecting model properties
    //============================================================================================
    void DashboardPageViewModel::reflect_mapper_ActiveSim(){
        using namespace gui::Models;
        auto sim = mapper.ActiveSim();
        update_property(sim_type, sim, L"SimIconSource");
        update_property(sim_icon_is_visible, sim != Simulators::none, L"SimIconIsVisible");
        if (sim == Simulators::none) {
            update_property(sim_name, std::wstring(L"No active flight simulator found"), L"SimString");
        }else if (sim == Simulators::fs2020){
            update_property(sim_name, std::wstring(L"Flight Simulator 2020"), L"SimString");
        }else if (sim == Simulators::dcs){
            update_property(sim_name, std::wstring(L"DCS world"), L"SimString");
        }
    }

    void DashboardPageViewModel::reflect_mapper_AircraftName(){
        auto name = mapper.AircraftName();
        if (name.size()){
            std::wstring value{L" : "};
            value += name;
            update_property(aircraft_name, value, L"SimString");
        }else{
            update_property(aircraft_name, std::wstring(name.c_str()), L"SimString");
        }
    }
}
