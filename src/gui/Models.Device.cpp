//
// Models.Device.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//
#include "pch.h"
#include "Models.Device.h"
#include "Models.Device.g.cpp"

namespace winrt::gui::Models::implementation
{
    Device::Device(hstring const& cname, hstring const& dname) : class_name(cname), device_name(dname){
    }

    hstring Device::ClassName(){
        return class_name;
    }
    
    hstring Device::DeviceName(){
        return device_name;
    }
}
