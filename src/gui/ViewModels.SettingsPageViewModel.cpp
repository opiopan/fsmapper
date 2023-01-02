//
// ViewModels.SettingsPageViewModel.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include "pch.h"
#include "ViewModels.SettingsPageViewModel.h"
#include "ViewModels.SettingsPageViewModel.g.cpp"

#include <thread>

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
}
