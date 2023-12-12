//
// ViewModels.SettingsPageViewModel.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once
#include "ViewModels.SettingsPageViewModel.g.h"
#include "config.hpp"
#include "encoding.hpp"
#include "tools.hpp"
#include <chrono>

namespace winrt::gui::ViewModels::implementation
{
    struct SettingsPageViewModel : SettingsPageViewModelT<SettingsPageViewModel>
    {
        SettingsPageViewModel() = default;

        bool RunScriptOnStartup(){
            return fsmapper::app_config.get_is_starting_script_at_start_up();
        }
        void RunScriptOnStartup(bool value){
            fsmapper::app_config.set_is_starting_script_at_start_up(value);
            save_config();
        }
        int32_t MessageLogSize(){
            return fsmapper::app_config.get_message_buffer_size();
        }
        void MessageLogSize(int32_t value){
            fsmapper::app_config.set_message_buffer_size(value);
            save_config();
        }
        hstring PreRunScript(){
            tools::utf8_to_utf16_translator str{fsmapper::app_config.get_pre_run_script()};
            return str.get();
        }
        void PreRunScript(hstring const& value){
            pre_run_script = value.c_str();
            fsmapper::app_config.set_pre_run_script(pre_run_script);
            save_config();
        }
        bool PreRunScriptIsVarid(){
            return fsmapper::app_config.get_pre_run_script_is_valid();
        }
        void PreRunScriptIsVarid(bool value){
            fsmapper::app_config.set_pre_run_script_is_valid(value);
            save_config();
        }
        int32_t RenderingMethod(){
            return fsmapper::app_config.get_rendering_method();
        }
        void RenderingMethod(int32_t value){
            if (value >= 0) {
                fsmapper::app_config.set_rendering_method(static_cast<MAPPER_OPTION_RENDERING_METHOD>(value));
                save_config();
            }
        }
        int32_t PluginFolderType(){
            return fsmapper::app_config.get_plugin_folder_is_default() ? 0 : 1;
        }
        void PluginFolderType(int32_t value){
            if (value == 1 && fsmapper::app_config.get_custom_plugin_folder().string().length() == 0){
                ClickChangePluginPathButton(nullptr, nullptr);
            }else if (value >= 0){
                fsmapper::app_config.set_plugin_folder_is_default(value == 0);
                save_config();
                notify_plugin_folder_changed();
            }
        }
        hstring DefaultPluginPath(){
            return fsmapper::app_config.get_default_plugin_folder().wstring().c_str();
        }
        winrt::Microsoft::UI::Xaml::Media::SolidColorBrush DefaultPluginPathColor(){
            auto key = fsmapper::app_config.get_plugin_folder_is_default() ?
                       L"ButtonForegroundThemeBrush" : L"ButtonDisabledForegroundThemeBrush";
            auto value = tools::ThemeResource(key);
            return value.as<winrt::Microsoft::UI::Xaml::Media::SolidColorBrush>();
        }
        hstring UserSpecifiedPluginPath(){
            return fsmapper::app_config.get_custom_plugin_folder().wstring().c_str();
        }
        winrt::Microsoft::UI::Xaml::Media::SolidColorBrush UserSpecifiedPluginPathColor(){
            auto key = !fsmapper::app_config.get_plugin_folder_is_default() ?
                       L"ButtonForegroundThemeBrush" : L"ButtonDisabledForegroundThemeBrush";
            auto value = tools::ThemeResource(key);
            return value.as<winrt::Microsoft::UI::Xaml::Media::SolidColorBrush>();
        }
        bool ChangePluginPathButtonIsValid(){
            return !fsmapper::app_config.get_plugin_folder_is_default();
        }

        winrt::Windows::Foundation::IAsyncAction ClickChangePluginPathButton(winrt::Windows::Foundation::IInspectable sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs args);

        winrt::event_token PropertyChanged(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler){
            return property_changed.add(handler);
        }
        void PropertyChanged(winrt::event_token const& token) noexcept{
            property_changed.remove(token);
        }

    protected:
        using clock = std::chrono::steady_clock;
        tools::utf16_to_utf8_translator pre_run_script;
        bool is_saving_config{false};
        int64_t update_count{0};
        clock::time_point last_update_time{clock::now()};

        winrt::Windows::Foundation::IAsyncAction save_config(bool updated = true);

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

        void notify_plugin_folder_changed(){
            using Args = Microsoft::UI::Xaml::Data::PropertyChangedEventArgs;
            property_changed(*this, Args{L"PluginFolderType"});
            property_changed(*this, Args{L"DefaultPluginPath"});
            property_changed(*this, Args{L"DefaultPluginPathColor"});
            property_changed(*this, Args{L"UserSpecifiedPluginPath"});
            property_changed(*this, Args{L"UserSpecifiedPluginPathColor"});
            property_changed(*this, Args{L"ChangePluginPathButtonIsValid"});
        }
    };
}
namespace winrt::gui::ViewModels::factory_implementation
{
    struct SettingsPageViewModel : SettingsPageViewModelT<SettingsPageViewModel, implementation::SettingsPageViewModel>
    {
    };
}
