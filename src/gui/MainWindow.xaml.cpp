#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif
#include <winrt/Windows.UI.Xaml.Interop.h>
#include "DashboardPage.xaml.h"
#include "ConsolePage.xaml.h"
#include "UtilitiesPage.xaml.h"
#include "SettingsPage.xaml.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;

namespace winrt::gui::implementation
{
    MainWindow::MainWindow()
    {
        InitializeComponent();
        //this->ExtendsContentIntoTitleBar(true);
        //this->SetTitleBar(AppTitleBar());
        this->Title(L"fsmapper");

        pages.emplace_back(page_data(L"dashboard", xaml_typename<gui::DashboardPage>()));
        pages.emplace_back(page_data(L"console", xaml_typename<gui::ConsolePage>()));
        pages.emplace_back(page_data(L"utilities", xaml_typename<gui::UtilitiesPage>()));
        pages.emplace_back(page_data(L"settings", xaml_typename<gui::SettingsPage>()));
    }

    void MainWindow::NavView_Loaded(
        Windows::Foundation::IInspectable const&,
        Microsoft::UI::Xaml::RoutedEventArgs const&)
    {
        restore_window_position();
        NavView().SelectedItem(NavView().MenuItems().GetAt(0));
    }

    void MainWindow::NavView_SelectionChanged(
        NavigationView const&,
        NavigationViewSelectionChangedEventArgs const& args)
    {
        if (args.SelectedItemContainer()) {
            auto tag = winrt::unbox_value_or<winrt::hstring>(
                args.SelectedItemContainer().Tag(), L"");
            for (auto&& page : pages) {
                if (page.first == tag) {
                    ContentFrame().Navigate(
                        page.second, nullptr, args.RecommendedNavigationTransitionInfo());
                }
            }
        }
    }

    void MainWindow::save_window_position()
    {
    }

    void MainWindow::restore_window_position()
    {
    }
}
