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
#include <sstream>

#include <winrt/Microsoft.UI.Xaml.Controls.h>

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
        token_for_mapper = mapper.PropertyChanged([this](auto const&, auto const& args){
            auto name = args.PropertyName();
            if (name == L"Status"){
                reflect_mapper_Status();
            }else if (name == L"ActiveSim"){
                reflect_mapper_ActiveSim();
            }else if (name == L"AircraftName"){
                reflect_mapper_AircraftName();
            }else if (name == L"MappingsInfo"){
                reflect_mapper_MappingsInfo();
            }else if (name == L"Viewports"){
                reflect_mapper_Viewports();
            }else if (name == L"Devices"){
                reflect_mapper_Devices();
            }else if (name == L"ViewportIsActive"){
                reflect_mapper_ViewportOperability();
            }else if (name == L"CapturedWindowStatus"){
                reflect_mapper_CapturedWindows();
            }
        });

        token_for_captured_windows = mapper.CapturedWindows().VectorChanged([this](auto const&, auto const&){
            reflect_mapper_CapturedWindows();
            reflect_mapper_ViewportOperability();
        });

        reflect_mapper_Status();
        reflect_mapper_ActiveSim();
        reflect_mapper_AircraftName();
        reflect_mapper_MappingsInfo();
        reflect_mapper_Viewports();
        reflect_mapper_Devices();
        reflect_mapper_CapturedWindows();
        reflect_mapper_ViewportOperability();
    }

    DashboardPageViewModel::~DashboardPageViewModel(){
        mapper.CapturedWindows().VectorChanged(token_for_captured_windows);
        mapper.PropertyChanged(token_for_mapper);
    }

    //============================================================================================
    // Viewport manipulation
    //============================================================================================
    winrt::Windows::Foundation::IAsyncAction DashboardPageViewModel::ToggleViewport(
        winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&){
        if (mapper.ViewportIsActive()){
            mapper.StopViewports();
        }else{
            if (!mapper.StartViewports()){
                winrt::Microsoft::UI::Xaml::Controls::ContentDialog dialog;
                dialog.Title(winrt::box_value(L"Error"));
                std::wostringstream os;
                dialog.Content(winrt::box_value(
                    L"Failed to enable viewports.\n"
                    L"Refere the message console for details."
                ));
                dialog.CloseButtonText(L"OK");
                dialog.XamlRoot(App::TopWindow().Content().XamlRoot());
                co_await dialog.ShowAsync();
            }
        }
    }
    

    //============================================================================================
    // Reflecting model properties
    //============================================================================================
    void DashboardPageViewModel::reflect_mapper_Status(){
        using namespace gui::Models;
        auto status = mapper.Status();
        update_property(normal_view_is_visible, status == MapperStatus::running, L"NormalViewIsVisible");
        update_property(stop_view_is_visible, status == MapperStatus::stop, L"StopViewIsVisible");
        update_property(error_view_is_visible, status == MapperStatus::error, L"ErrorViewIsVisible");
    }

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

    void DashboardPageViewModel::reflect_mapper_MappingsInfo(){
        auto stat = mapper.MappingsInfo();
        auto sum = stat.Primery() + stat.Secondary() + stat.Viewports() + stat.Views();
        update_property(mappings_detail_is_visible, sum > 0, L"MappingsDetailIsVisible");
        update_property(mappings_detail_primery, stat.Primery(), L"MappingsDetailPrimery");
        update_property(mappings_detail_secondary, stat.Secondary(), L"MappingsDetailSecondary");
        update_property(mappings_detail_viewports, stat.Viewports(), L"MappingsDetailViewports");
        update_property(mappings_detail_views, stat.Views(), L"MappingsDetailViews");
        std::wostringstream os;
        if (sum == 0) {
            os << L"No event-action mapping is registerd.";
        }else if (sum == 1){
            os << L"1 event-action mapping is registerd:";
        }else{
            os << sum << " event-action mappings are registerd:";
        }
        update_property(mappings_summary, std::move(hstring(std::move(os.str()))), L"MappingsSummary");
    }

    void DashboardPageViewModel::reflect_mapper_Viewports(){
        auto viewports = mapper.Viewports();
        auto size = viewports.Size();
        std::wostringstream os_summary;
        if (size == 0) {
            os_summary << L"No viewport found.";
        }else if (size == 1){
            os_summary << L"1 viewport is defined:";
        }else{
            os_summary << size << L" viewports are defined:";
        }
        update_property(viewport_summary, std::move(hstring(os_summary.str())), L"ViewportSummary");

        std::wostringstream os_detail;
        const wchar_t* sep = L"";
        for (auto const& viewport: viewports){
            os_detail << sep << "- " << viewport.Name().c_str() << "  [";
            auto viewnum = viewport.Views().Size();
            if (viewnum == 0) {
                os_detail << L"with no view]";
            }else if (viewnum == 1){
                os_detail << L"contains 1 view]";
            }else{
                os_detail << L"contains " << viewnum << L"views]";
            }
            sep = L"\n";
        }
        update_property(viewport_detail, std::move(hstring(os_detail.str())), L"ViewportDetail");
        update_property(viewport_detail_is_visible, size > 0, L"ViewportDetailIsVisible");
    }


    void DashboardPageViewModel::reflect_mapper_Devices(){
        auto devices = mapper.Devices();
        auto size = devices.Size();
        hstring summary;
        if (size == 0) {
            summary = L"No opend device found.";
        }else if (size == 1){
            summary = L"1 device is opend:";
        }else{
            std::wostringstream os;
            os << size << L" devices are opend:";
            summary = std::move(os.str());
        }
        update_property(device_summary, std::move(summary), L"DeviceSummary");

        std::wostringstream os;
        const wchar_t* sep = L"";
        for (auto const& device : devices) {
            os << sep << L"- " << device.DeviceName().c_str() << L"  [" << device.ClassName().c_str() << L"]";
            sep = L"\n";
        }
        update_property(device_detail, std::move(hstring(os.str())), L"DeviceDetail");
        update_property(device_detail_is_visible, devices.Size() > 0, L"DeviceDetailIsVisible");
    }

    void DashboardPageViewModel::reflect_mapper_CapturedWindows(){
        auto cw_num = mapper.CapturedWindows().Size();
        std::wostringstream os;
        auto captured_num = 0;
        for (auto const& cw : mapper.CapturedWindows()) {
            captured_num += cw.IsCaptured() ? 1 : 0;
        }
        if (cw_num == 1) {
            os << "1 captured window is defined (";
        }else if (cw_num > 1){
            os << cw_num << " captured windows are defined (";
        }
        os << captured_num << "/" << cw_num << " captured):";
        update_property(captured_windows_summary, std::move(hstring(os.str())), L"CapturedWindowsSummary");
        update_property(captured_windows_is_visible, cw_num > 0, L"CapturedWindowsIsVisible");
    }

    void DashboardPageViewModel::reflect_mapper_ViewportOperability(){
        auto viewport_is_active = mapper.ViewportIsActive();
        auto cw_is_registerd = mapper.CapturedWindows().Size() > 0;
        auto button_text = viewport_is_active ? L"Stop Viewports" : L"Activate Viewports";
        update_property(viewport_button_text, std::move(hstring(button_text)), L"ViewportButtonText");
        update_property(viewport_button_is_enabled, cw_is_registerd, L"ViewportButtonIsEnabled");
        update_property(viewport_button_is_visible, cw_is_registerd, L"ViewportButtonIsVisible");
        update_property(captured_window_button_is_enabled, !viewport_is_active, L"CapturedWindowButtonIsEnabled");
    }
}
