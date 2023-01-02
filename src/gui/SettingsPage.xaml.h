#pragma once

#include "SettingsPage.g.h"

namespace winrt::gui::implementation
{
    struct SettingsPage : SettingsPageT<SettingsPage>
    {
        SettingsPage();

        winrt::gui::ViewModels::SettingsPageViewModel ViewModel() {
            return view_model;
        }

    protected:
        winrt::gui::ViewModels::SettingsPageViewModel view_model{nullptr};
    };
}

namespace winrt::gui::factory_implementation
{
    struct SettingsPage : SettingsPageT<SettingsPage, implementation::SettingsPage>
    {
    };
}
