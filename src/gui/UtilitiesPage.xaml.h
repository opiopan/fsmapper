#pragma once

#include "UtilitiesPage.g.h"

namespace winrt::gui::implementation
{
    struct UtilitiesPage : UtilitiesPageT<UtilitiesPage>
    {
        UtilitiesPage();
    };
}

namespace winrt::gui::factory_implementation
{
    struct UtilitiesPage : UtilitiesPageT<UtilitiesPage, implementation::UtilitiesPage>
    {
    };
}
