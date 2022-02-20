#pragma once

#include "App.xaml.g.h"
#include "Models.Mapper.h"

namespace winrt::gui::implementation
{
    struct App : AppT<App>
    {
        App();

        void OnLaunched(Microsoft::UI::Xaml::LaunchActivatedEventArgs const&);
        
        static winrt::gui::Models::Mapper Mapper(){return mapper;}

    private:
        winrt::Microsoft::UI::Xaml::Window window{nullptr};
        static winrt::gui::Models::Mapper mapper;
    };
}
