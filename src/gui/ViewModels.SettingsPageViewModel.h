//
// ViewModels.SettingsPageViewModel.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once
#include "ViewModels.SettingsPageViewModel.g.h"
#include "config.hpp"
#include "encoding.hpp"
#include "tools.hpp"
#include "../.version.h"
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
        bool LuaLibBaseIsEnable(){
            return fsmapper::app_config.get_lua_standard_libraries() & MOPT_STDLIB_BASE;
        }
        void LuaLibBaseIsEnable(bool value){
            auto current = fsmapper::app_config.get_lua_standard_libraries() & ~MOPT_STDLIB_BASE;
            fsmapper::app_config.set_lua_standard_libraries(current | (value ? MOPT_STDLIB_BASE : 0));
            save_config();
        }
        bool LuaLibCoroutineIsEnable(){
            return fsmapper::app_config.get_lua_standard_libraries() & MOPT_STDLIB_COROUTINE;
        }
        void LuaLibCoroutineIsEnable(bool value){
            auto current = fsmapper::app_config.get_lua_standard_libraries() & ~MOPT_STDLIB_COROUTINE;
            fsmapper::app_config.set_lua_standard_libraries(current | (value ? MOPT_STDLIB_COROUTINE : 0));
            save_config();
        }
        bool LuaLibDebugIsEnable(){
            return fsmapper::app_config.get_lua_standard_libraries() & MOPT_STDLIB_DEBUG;
        }
        void LuaLibDebugIsEnable(bool value){
            auto current = fsmapper::app_config.get_lua_standard_libraries() & ~MOPT_STDLIB_DEBUG;
            fsmapper::app_config.set_lua_standard_libraries(current | (value ? MOPT_STDLIB_DEBUG : 0));
            save_config();
        }
        bool LuaLibIoIsEnable(){
            return fsmapper::app_config.get_lua_standard_libraries() & MOPT_STDLIB_IO;
        }
        void LuaLibIoIsEnable(bool value){
            auto current = fsmapper::app_config.get_lua_standard_libraries() & ~MOPT_STDLIB_IO;
            fsmapper::app_config.set_lua_standard_libraries(current | (value ? MOPT_STDLIB_IO : 0));
            save_config();
        }
        bool LuaLibMathIsEnable(){
            return fsmapper::app_config.get_lua_standard_libraries() & MOPT_STDLIB_MATH;
        }
        void LuaLibMathIsEnable(bool value){
            auto current = fsmapper::app_config.get_lua_standard_libraries() & ~MOPT_STDLIB_MATH;
            fsmapper::app_config.set_lua_standard_libraries(current | (value ? MOPT_STDLIB_MATH : 0));
            save_config();
        }
        bool LuaLibOsIsEnable(){
            return fsmapper::app_config.get_lua_standard_libraries() & MOPT_STDLIB_OS;
        }
        void LuaLibOsIsEnable(bool value){
            auto current = fsmapper::app_config.get_lua_standard_libraries() & ~MOPT_STDLIB_OS;
            fsmapper::app_config.set_lua_standard_libraries(current | (value ? MOPT_STDLIB_OS : 0));
            save_config();
        }
        bool LuaLibPackageIsEnable(){
            return fsmapper::app_config.get_lua_standard_libraries() & MOPT_STDLIB_PACKAGE;
        }
        void LuaLibPackageIsEnable(bool value){
            auto current = fsmapper::app_config.get_lua_standard_libraries() & ~MOPT_STDLIB_PACKAGE;
            fsmapper::app_config.set_lua_standard_libraries(current | (value ? MOPT_STDLIB_PACKAGE : 0));
            save_config();
        }
        bool LuaLibStringIsEnable(){
            return fsmapper::app_config.get_lua_standard_libraries() & MOPT_STDLIB_STRING;
        }
        void LuaLibStringIsEnable(bool value){
            auto current = fsmapper::app_config.get_lua_standard_libraries() & ~MOPT_STDLIB_STRING;
            fsmapper::app_config.set_lua_standard_libraries(current | (value ? MOPT_STDLIB_STRING : 0));
            save_config();
        }
        bool LuaLibTableIsEnable(){
            return fsmapper::app_config.get_lua_standard_libraries() & MOPT_STDLIB_TABLE;
        }
        void LuaLibTableIsEnable(bool value){
            auto current = fsmapper::app_config.get_lua_standard_libraries() & ~MOPT_STDLIB_TABLE;
            fsmapper::app_config.set_lua_standard_libraries(current | (value ? MOPT_STDLIB_TABLE : 0));
            save_config();
        }
        bool LuaLibUtf8IsEnable(){
            return fsmapper::app_config.get_lua_standard_libraries() & MOPT_STDLIB_UTF8;
        }
        void LuaLibUtf8IsEnable(bool value){
            auto current = fsmapper::app_config.get_lua_standard_libraries() & ~MOPT_STDLIB_UTF8;
            fsmapper::app_config.set_lua_standard_libraries(current | (value ? MOPT_STDLIB_UTF8 : 0));
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
        hstring PluginPath(){
            return fsmapper::app_config.get_plugin_folder_is_default() ? fsmapper::app_config.get_default_plugin_folder().wstring().c_str() :
                                                                         fsmapper::app_config.get_custom_plugin_folder().wstring().c_str();
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
        hstring Version(){
            return L"v" VERSTR_TITLE_VERSION;
        }
        hstring Copyright(){
            return L"©︎ " COPYRIGHT_STR " Hiroshi Murayama, All rights reserved.";
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
            property_changed(*this, Args{L"PluginPath"});
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
