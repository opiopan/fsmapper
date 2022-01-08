#include "pch.h"
#include "UtilitiesPage.xaml.h"
#if __has_include("UtilitiesPage.g.cpp")
#include "UtilitiesPage.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::gui::implementation
{
    UtilitiesPage::UtilitiesPage()
    {
        InitializeComponent();
    }
}
