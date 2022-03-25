//
// ViewModels.ConsolePageViewModel.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include "pch.h"
#include "ViewModels.ConsolePageViewModel.h"
#include "ViewModels.ConsolePageViewModel.g.cpp"

#include "App.xaml.h"

using App = winrt::gui::implementation::App;

namespace winrt::gui::ViewModels::implementation{
    //============================================================================================
    // Object constructor / destructor
    //============================================================================================
    ConsolePageViewModel::ConsolePageViewModel(){
        mapper = App::Mapper();
    }

    ConsolePageViewModel::~ConsolePageViewModel(){}
}
