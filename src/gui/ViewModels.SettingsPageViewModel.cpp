//
// ViewModels.SettingsPageViewModel.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include "pch.h"
#include "ViewModels.SettingsPageViewModel.h"
#include "ViewModels.SettingsPageViewModel.g.cpp"
#include "App.xaml.h"

#include <thread>
#include <shobjidl_core.h>
#include <winrt/windows.storage.pickers.h>

using namespace winrt;
using namespace winrt::Microsoft::UI::Xaml;
using App = winrt::gui::implementation::App;

namespace winrt::gui::ViewModels::implementation
{
    SettingsPageViewModel::SettingsPageViewModel(){
        mapper = App::Mapper();
        dcs_installer.check();
    }

    winrt::Windows::Foundation::IAsyncAction SettingsPageViewModel::save_config(bool updated){
        if (updated){
            update_count++;
            last_update_time = clock::now();
        }
        if (!is_saving_config){
            is_saving_config = true;
            auto count = update_count;
            auto strong_this{get_strong()};
            winrt::apartment_context ui_thread;
            co_await winrt::resume_background();
            std::this_thread::sleep_until(last_update_time + std::chrono::seconds(5));
            co_await ui_thread;
            is_saving_config = false;
            if (count == update_count){
                fsmapper::app_config.save();
            }else{
                save_config(false);
            }
        }
    }

    winrt::Windows::Foundation::IAsyncAction SettingsPageViewModel::ClickChangePluginPathButton(
        winrt::Windows::Foundation::IInspectable, winrt::Microsoft::UI::Xaml::RoutedEventArgs){
        auto picker = winrt::Windows::Storage::Pickers::FolderPicker();
        picker.as<IInitializeWithWindow>()->Initialize(App::TopWindowHandle());
        auto folder = co_await picker.PickSingleFolderAsync();
        if (folder){
            std::filesystem::path path{folder.Path().c_str()};
            fsmapper::app_config.set_custom_plugin_folder(std::move(path));
        }
        fsmapper::app_config.set_plugin_folder_is_default(
            fsmapper::app_config.get_custom_plugin_folder().string().size() == 0
        );
        save_config();
        notify_plugin_folder_changed();
    }

    void SettingsPageViewModel::ClickDownloadNewReleaseButton(winrt::Windows::Foundation::IInspectable, winrt::Microsoft::UI::Xaml::RoutedEventArgs){
        mapper.DownloadLatestRelease();
    }

    winrt::Windows::Foundation::IAsyncAction SettingsPageViewModel::CheckAndInstallDcsExporter(bool mode){
        auto result = co_await dcs::confirm_change_export_lua(
            mode ? fsmapper::config::dcs_exporter_mode::on : fsmapper::config::dcs_exporter_mode::off, 
            dcs_installer);
        if (result == dcs::confirmation_yes){
            if (!dcs_installer.install(mode ? dcs::installer::mode::install : dcs::installer::mode::uninstall)){
                dcs_installer.show_install_error();
                co_return;
            }
        }
        if (mode){
            auto next = result == dcs::confirmation_yes     ? fsmapper::config::dcs_exporter_mode::on :
                        result == dcs::no_changes_needed    ? fsmapper::config::dcs_exporter_mode::on :
                                                              fsmapper::config::dcs_exporter_mode::off;
            fsmapper::app_config.set_dcs_exporter_mode(next);
        }else{
            auto next = result == dcs::confirmation_yes     ? fsmapper::config::dcs_exporter_mode::off :
                        result == dcs::no_changes_needed    ? fsmapper::config::dcs_exporter_mode::off :
                                                              fsmapper::config::dcs_exporter_mode::on;
            fsmapper::app_config.set_dcs_exporter_mode(next);
        }
        dcs_installer.check();
        update_property(L"DcsExporterIsEnabled");
    }

}
