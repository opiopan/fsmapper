#pragma once

#include "ConsolePage.g.h"

namespace winrt::gui::implementation
{
    struct ConsolePage : ConsolePageT<ConsolePage>
    {
        ConsolePage();

        int32_t MyProperty();
        void MyProperty(int32_t value);

        void myButton_Click(Windows::Foundation::IInspectable const& sender, Microsoft::UI::Xaml::RoutedEventArgs const& args);
    };
}

namespace winrt::gui::factory_implementation
{
    struct ConsolePage : ConsolePageT<ConsolePage, implementation::ConsolePage>
    {
    };
}
