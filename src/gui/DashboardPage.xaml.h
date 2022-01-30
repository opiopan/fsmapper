#pragma once

#include "DashboardPage.g.h"

namespace winrt::gui::implementation
{
    struct DashboardPage : DashboardPageT<DashboardPage>
    {
        DashboardPage();

        int32_t MyProperty();
        void MyProperty(int32_t value);
    };
}

namespace winrt::gui::factory_implementation
{
    struct DashboardPage : DashboardPageT<DashboardPage, implementation::DashboardPage>
    {
    };
}
