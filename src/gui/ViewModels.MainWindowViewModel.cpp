//
// ViewModels.MainWindowViewModel.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//
#include "pch.h"
#include "ViewModels.MainWindowViewModel.h"
#include "ViewModels.MainWindowViewModel.g.cpp"
#include "App.xaml.h"
#include "tools.hpp"

#include <filesystem>
#include <chrono>
#include <shobjidl.h>
#include <winrt/windows.storage.pickers.h>

using namespace winrt;
using namespace winrt::Microsoft::UI::Xaml;
using App = winrt::gui::implementation::App;

namespace winrt::gui::ViewModels::implementation{
    //============================================================================================
    // Object constructor / destructor
    //============================================================================================
    MainWindowViewModel::MainWindowViewModel(){
        status_brushes[static_cast<int>(StatusBrush::stop)] = 
            unbox_value<Media::SolidColorBrush>(tools::AppResource(L"ScriptStatusColorStop"));
        status_brushes[static_cast<int>(StatusBrush::running)] = 
            unbox_value<Media::SolidColorBrush>(tools::AppResource(L"ScriptStatusColorRunning"));
        status_brushes[static_cast<int>(StatusBrush::error_light)] = 
            unbox_value<Media::SolidColorBrush>(tools::AppResource(L"ScriptStatusColorErrorLight"));
        status_brushes[static_cast<int>(StatusBrush::error_dark)] = 
            unbox_value<Media::SolidColorBrush>(tools::AppResource(L"ScriptStatusColorErrorDark"));
     
        mapper = App::Mapper();
        token_for_mapper = mapper.PropertyChanged([this](auto const&, auto const& args) {
            auto name = args.PropertyName();
            if (name == L"ScriptPath"){
                reflect_mapper_ScriptPath();
            }else if (name == L"Status"){
                reflect_mapper_Status();
            }
        });
     
        reflect_mapper_ScriptPath();
        reflect_mapper_Status();
    }

    MainWindowViewModel::~MainWindowViewModel(){
        mapper.PropertyChanged(token_for_mapper);
    }

    //============================================================================================
    // Properties of runtime class
    //============================================================================================
    hstring MainWindowViewModel::ScriptName(){
        return script_name;
    }

    winrt::Microsoft::UI::Xaml::Media::SolidColorBrush MainWindowViewModel::ScriptStatusBrush(){
        return status_brushes[static_cast<int>(script_status_brush)];
    }

    bool MainWindowViewModel::OpenButtonIsEnabled(){
        return open_button_is_enabled;
    }

    bool MainWindowViewModel::StartStopButtonIsEnabled(){
        return start_stop_button_is_enabled;
    }

    hstring MainWindowViewModel::StartStopButtonIcon(){
        return start_stop_button_icon;
    }

    hstring MainWindowViewModel::StartStopButtonLabel(){
        return start_stop_button_label;
    }

    //============================================================================================
    // event handlers for binding
    //============================================================================================
    winrt::Windows::Foundation::IAsyncAction MainWindowViewModel::ClickOpenButton(
        winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args){
        auto picker = winrt::Windows::Storage::Pickers::FileOpenPicker();
        picker.as<IInitializeWithWindow>()->Initialize(App::TopWindowHandle());
        picker.FileTypeFilter().Append(L".lua");
        auto file = co_await picker.PickSingleFileAsync();
        if (file) {
            mapper.ScriptPath(file.Path());
            mapper.RunScript();
        }
    }

    void MainWindowViewModel::ClickStartStopButton(
        winrt::Windows::Foundation::IInspectable const&, winrt::Microsoft::UI::Xaml::RoutedEventArgs const&){
        if (mapper.Status() == gui::Models::MapperStatus::running) {
            mapper.StopScript();
        }else{
            mapper.RunScript();
        }
    }

    //============================================================================================
    // Event notification
    //============================================================================================
    winrt::event_token MainWindowViewModel::PropertyChanged(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler){
        return property_changed.add(handler);
    }

    void MainWindowViewModel::PropertyChanged(winrt::event_token const& token) noexcept{
        return property_changed.remove(token);
    }

    //============================================================================================
    // Reflecting model properties
    //============================================================================================
    void MainWindowViewModel::reflect_mapper_ScriptPath(){
        auto path_str{mapper.ScriptPath()};
        if (path_str.size()) {
            std::filesystem::path path{path_str.c_str()};
            update_property(script_name, hstring(path.filename().c_str()), L"ScriptName");
            update_property(start_stop_button_is_enabled, true, L"StartStopButtonIsEnabled");
        }else{
            update_property(script_name, hstring(L""), L"ScriptName");
            update_property(start_stop_button_is_enabled, false, L"StartStopButtonIsEnabled");
        }
    }

    void MainWindowViewModel::reflect_mapper_Status(){
        auto status = mapper.Status();

        auto inactive = status != gui::Models::MapperStatus::running;
        update_property(open_button_is_enabled, inactive, L"OpenButtonIsEnabled");
        update_property(start_stop_button_icon, hstring(inactive ? L"\xe768" : L"\xe71A"), L"StartStopButtonIcon");
        update_property(start_stop_button_label, hstring(inactive ? L"Run" : L"Stop"), L"StartStopButtonLabel");

        if (status == gui::Models::MapperStatus::stop){
            error_flash_thread_counter++;
            update_property(script_status_brush, StatusBrush::stop, L"ScriptStatusBrush");
        }else if (status == gui::Models::MapperStatus::running){
            error_flash_thread_counter++;
            update_property(script_status_brush, StatusBrush::running, L"ScriptStatusBrush");
        }else if (status == gui::Models::MapperStatus::error){
            if (script_status_brush == StatusBrush::running || script_status_brush == StatusBrush::stop){
                auto flasher = [this]() -> Windows::Foundation::IAsyncAction{
                    auto self = this;
                    winrt::apartment_context ui_thread;
                    auto counter = error_flash_thread_counter;
                    while (true){
                        self->update_property(self->script_status_brush, StatusBrush::error_light, L"ScriptStatusBrush");
                        co_await winrt::resume_background();
                        std::this_thread::sleep_for(std::chrono::milliseconds(300));
                        co_await ui_thread;
                        if (self->error_flash_thread_counter != counter){
                            break;
                        }
                        self->update_property(self->script_status_brush, StatusBrush::error_dark, L"ScriptStatusBrush");
                        co_await winrt::resume_background();
                        std::this_thread::sleep_for(std::chrono::milliseconds(700));
                        co_await ui_thread;
                        if (self->error_flash_thread_counter != counter){
                            break;
                        }
                    }
                    co_return;
                };
                error_flasher = flasher();
            }
        }
    }
}
