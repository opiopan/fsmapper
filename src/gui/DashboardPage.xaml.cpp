//
// DashboardPage.xml.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//
#include "pch.h"
#include "DashboardPage.xaml.h"
#if __has_include("DashboardPage.g.cpp")
#include "DashboardPage.g.cpp"
#endif

#include "ViewModels.DashboardPageViewModel.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;

namespace winrt::gui::implementation
{
    DashboardPage::DashboardPage(){
        view_model  = winrt::make<gui::ViewModels::implementation::DashboardPageViewModel>();
        InitializeComponent();
    }
}
