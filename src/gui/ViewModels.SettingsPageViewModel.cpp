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
}
