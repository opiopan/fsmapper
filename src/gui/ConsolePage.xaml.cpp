#include "pch.h"
#include "ConsolePage.xaml.h"
#if __has_include("ConsolePage.g.cpp")
#include "ConsolePage.g.cpp"
#endif

#include "ViewModels.ConsolePageViewModel.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;

namespace winrt::gui::implementation
{
    ConsolePage::ConsolePage(){
        view_model = winrt::make<gui::ViewModels::implementation::ConsolePageViewModel>();
        InitializeComponent();
    }
}
