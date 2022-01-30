#include "pch.h"
#include "DashboardPage.xaml.h"
#if __has_include("DashboardPage.g.cpp")
#include "DashboardPage.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace winrt::gui::implementation
{
    DashboardPage::DashboardPage()
    {
        InitializeComponent();
    }

    int32_t DashboardPage::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void DashboardPage::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }
}
