#pragma once

#include "App.xaml.g.h"
#include "Models.h"

namespace winrt::gui::implementation
{
    struct App : AppT<App>
    {
        App();

        void OnLaunched(Microsoft::UI::Xaml::LaunchActivatedEventArgs const&);
        
        static winrt::Microsoft::UI::Xaml::Window TopWindow(){return window;}
        static HWND TopWindowHandle();
        static winrt::gui::Models::Mapper Mapper(){return mapper;}

    private:
        static winrt::Microsoft::UI::Xaml::Window window;
        static winrt::gui::Models::Mapper mapper;
    };
}
