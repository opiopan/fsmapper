#include "pch.h"
#include "SettingsPage.xaml.h"
#if __has_include("SettingsPage.g.cpp")
#include "SettingsPage.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;

#include "ViewModels.SettingsPageViewModel.h"

namespace winrt::gui::implementation
{
    SettingsPage::SettingsPage()
    {
        view_model = winrt::make<gui::ViewModels::implementation::SettingsPageViewModel>();
        InitializeComponent();
    }
}
