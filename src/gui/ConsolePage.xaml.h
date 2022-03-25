#pragma once

#include "ConsolePage.g.h"

namespace winrt::gui::implementation
{
    struct ConsolePage : ConsolePageT<ConsolePage>
    {
        ConsolePage();

        winrt::gui::ViewModels::ConsolePageViewModel ViewModel() {
            return view_model;
        }

    protected:
        winrt::gui::ViewModels::ConsolePageViewModel view_model{ nullptr };
    };
}

namespace winrt::gui::factory_implementation
{
    struct ConsolePage : ConsolePageT<ConsolePage, implementation::ConsolePage>
    {
    };
}
