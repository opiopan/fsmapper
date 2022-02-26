//
// DashboardPage.xml.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//
#pragma once

#include "DashboardPage.g.h"

namespace winrt::gui::implementation
{
    struct DashboardPage : DashboardPageT<DashboardPage>{
        DashboardPage();

        winrt::gui::ViewModels::DashboardPageViewModel ViewModel(){
            return view_model;
        }

    protected:
        winrt::gui::ViewModels::DashboardPageViewModel view_model{nullptr};
    };
}

namespace winrt::gui::factory_implementation
{
    struct DashboardPage : DashboardPageT<DashboardPage, implementation::DashboardPage>
    {
    };
}
