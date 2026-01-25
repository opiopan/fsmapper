//
// ViewModels.SettingsPageViewModel.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once
#include "ViewModels.SettingsPageViewModel.g.h"
#include "Models.h"
#include "config.hpp"
#include "encoding.hpp"
#include "tools.hpp"
#include "dcs_installer.hpp"
#include "version_parser.hpp"
#include "../.version.h"
#include <chrono>
#include <format>

namespace winrt::gui::ViewModels::implementation
{
    struct SettingsPageViewModel : SettingsPageViewModelT<SettingsPageViewModel>
    {
        SettingsPageViewModel();

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
        bool DeveloperLog(){
            return fsmapper::app_config.get_developer_log();
        }
        void DeveloperLog(bool value){
            fsmapper::app_config.set_developer_log(value);
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
        bool PreRunScriptIsValid(){
            return fsmapper::app_config.get_pre_run_script_is_valid();
        }
        void PreRunScriptIsValid(bool value){
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
        bool UseSeparatedUIThread(){
            return fsmapper::app_config.get_use_separated_ui_thread();
        }
        void UseSeparatedUIThread(bool value){
            fsmapper::app_config.set_use_separated_ui_thread(value);
            save_config();
        }
        hstring PluginSearchPath(){
            if (fsmapper::app_config.get_plugin_folder_is_default()) {
                return format(L"{0}\n{1}",
                    fsmapper::app_config.get_default_plugin_folder2().wstring().c_str(),
                    fsmapper::app_config.get_default_plugin_folder().wstring().c_str()).c_str();
            }else{
                return format(L"{0}\n{1}\n{2}",
                    fsmapper::app_config.get_custom_plugin_folder().wstring().c_str(),
                    fsmapper::app_config.get_default_plugin_folder2().wstring().c_str(),
                    fsmapper::app_config.get_default_plugin_folder().wstring().c_str()).c_str();
            }
        }
        bool UserPluginPathIsEnable(){
            return !fsmapper::app_config.get_plugin_folder_is_default();
        }
        void UserPluginPathIsEnable(bool value){
            if (value && fsmapper::app_config.get_custom_plugin_folder().string().length() == 0){
                ClickChangePluginPathButton(nullptr, nullptr);
            }else{
                fsmapper::app_config.set_plugin_folder_is_default(!value);
                save_config();
                notify_plugin_folder_changed();
            }
        }
        hstring UserPluginPath(){
            return fsmapper::app_config.get_custom_plugin_folder().wstring().c_str();
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
        hstring Commit(){
            return L"" COMMIT_STR;
        }
        hstring CommitUrl(){
            return L"https://github.com/opiopan/fsmapper/tree/" COMMIT_STR "/src";
        }
        bool NewReleaseAvailable(){
            utils::parsed_version current{mapper.LatestRelease().c_str()};
            return current > utils::this_version;
        }
        hstring NewRelease(){
            return mapper.LatestRelease();
        }
        bool DcsIsInstalled(){
            return dcs_installer.status != dcs::exporter_status::unknown &&
                   dcs_installer.status != dcs::exporter_status::no_dcs;
        }
        bool DcsExporterIsEnabled(){
            return fsmapper::app_config.get_dcs_exporter_mode() == fsmapper::config::dcs_exporter_mode::on;
        }
        void DcsExporterIsEnabled(bool value){
            CheckAndInstallDcsExporter(value);
        }
        bool TouchMouseEmulationIsEnable(){
            return fsmapper::app_config.get_touch_mouse_emulation_is_enable();
        }
        void TouchMouseEmulationIsEnable(bool value){
            fsmapper::app_config.set_touch_mouse_emulation_is_enable(value);
            save_config();
        }
        bool TouchDelayMouseEmulation(){
            return fsmapper::app_config.get_touch_delay_mouse_emulation();
        }
        void TouchDelayMouseEmulation(bool value){
            fsmapper::app_config.set_touch_delay_mouse_emulation(value);
            save_config();
        }
        uint32_t TouchCursorDelay(){
            return fsmapper::app_config.get_touch_cursor_delay();
        }
        void TouchCursorDelay(int32_t value){
            fsmapper::app_config.set_touch_cursor_delay(value);
            save_config();
        }
        uint32_t TouchDownDelay(){
            return fsmapper::app_config.get_touch_down_delay();
        }
        void TouchDownDelay(int32_t value){
            fsmapper::app_config.set_touch_down_delay(value);
            save_config();
        }
        int32_t TouchUpDelay(){
            return fsmapper::app_config.get_touch_up_delay();
        }
        void TouchUpDelay(int32_t value){
            fsmapper::app_config.set_touch_up_delay(value);
            save_config();
        }
        int32_t TouchDragStartDelay(){
            return fsmapper::app_config.get_touch_drag_start_delay();
        }
        void TouchDragStartDelay(int32_t value){
            fsmapper::app_config.set_touch_drag_start_delay(value);
            save_config();
        }
        bool TouchDoubleTapOnDrag(){
            return fsmapper::app_config.get_touch_double_tap_on_drag();
        }
        void TouchDoubleTapOnDrag(bool value){
            fsmapper::app_config.set_touch_double_tap_on_drag(value);
            save_config();
        }
        int32_t TouchDeadzoneForDrag(){
            return fsmapper::app_config.get_touch_deadzone_for_drag();
        }
        void TouchDeadzoneForDrag(int32_t value){
            fsmapper::app_config.set_touch_deadzone_for_drag(value);
            save_config();
        }
        int32_t TouchPointerJitter(){
            return fsmapper::app_config.get_touch_pointer_jitter();
        }
        void TouchPointerJitter(int32_t value){
            fsmapper::app_config.set_touch_pointer_jitter(value);
            save_config();
        }
        int32_t TouchMoveTriggerDistance(){
            return fsmapper::app_config.get_touch_move_trigger_distance();
        }
        void TouchMoveTriggerDistance(int32_t value){
            fsmapper::app_config.set_touch_move_trigger_distance(value);
            save_config();
        }
        int32_t TouchMinimumInterval(){
            return fsmapper::app_config.get_touch_minimum_interval();
        }
        void TouchMinimumInterval(int32_t value){
            fsmapper::app_config.set_touch_minimum_interval(value);
            save_config();
        }

        winrt::Windows::Foundation::IAsyncAction ClickChangePluginPathButton(winrt::Windows::Foundation::IInspectable sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs args);
        void ClickResetTouchConfig(winrt::Windows::Foundation::IInspectable sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs args);
        void ClickResetTouchConfigForRemote(winrt::Windows::Foundation::IInspectable sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs args);
        void ClickDownloadNewReleaseButton(winrt::Windows::Foundation::IInspectable sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs args);

        winrt::event_token PropertyChanged(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler){
            return property_changed.add(handler);
        }
        void PropertyChanged(winrt::event_token const& token) noexcept{
            property_changed.remove(token);
        }

    protected:
        winrt::gui::Models::Mapper mapper{nullptr};
        using clock = std::chrono::steady_clock;
        tools::utf16_to_utf8_translator pre_run_script;
        bool is_saving_config{false};
        int64_t update_count{0};
        clock::time_point last_update_time{clock::now()};
        dcs::installer dcs_installer;

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
            property_changed(*this, Args{L"PluginSearchPath"});
            property_changed(*this, Args{L"UserPluginPathIsEnable"});
            property_changed(*this, Args{L"UserPluginPath"});
            property_changed(*this, Args{L"DefaultPluginPath"});
            property_changed(*this, Args{L"DefaultPluginPathColor"});
            property_changed(*this, Args{L"UserSpecifiedPluginPath"});
            property_changed(*this, Args{L"UserSpecifiedPluginPathColor"});
            property_changed(*this, Args{L"ChangePluginPathButtonIsValid"});
        }

        void notify_touch_config_changed(){
            using Args = Microsoft::UI::Xaml::Data::PropertyChangedEventArgs;
            property_changed(*this, Args{L"TouchDelayMouseEmulation"});
            property_changed(*this, Args{L"TouchCursorDelay"});
            property_changed(*this, Args{L"TouchDownDelay"});
            property_changed(*this, Args{L"TouchUpDelay"});
            property_changed(*this, Args{L"TouchDragStartDelay"});
            property_changed(*this, Args{L"TouchDoubleTapOnDrag"});
            property_changed(*this, Args{L"TouchDeadzoneForDrag"});
            property_changed(*this, Args{L"TouchPointerJitter"});
            property_changed(*this, Args{L"TouchMoveTriggerDistance"});
            property_changed(*this, Args{L"TouchMinimumInterval"});
        }

        winrt::Windows::Foundation::IAsyncAction CheckAndInstallDcsExporter(bool mode);
    };
}
namespace winrt::gui::ViewModels::factory_implementation
{
    struct SettingsPageViewModel : SettingsPageViewModelT<SettingsPageViewModel, implementation::SettingsPageViewModel>
    {
    };
}
