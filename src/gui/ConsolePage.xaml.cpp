﻿#include "pch.h"
#include "ConsolePage.xaml.h"
#if __has_include("ConsolePage.g.cpp")
#include "ConsolePage.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::gui::implementation
{
    ConsolePage::ConsolePage()
    {
        InitializeComponent();
    }

    int32_t ConsolePage::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void ConsolePage::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }

    void ConsolePage::myButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        myButton().Content(box_value(L"Clicked"));
    }
}