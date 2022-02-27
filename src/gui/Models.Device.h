//
// Models.Device.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//
#pragma once
#include "Models.Device.g.h"

namespace winrt::gui::Models::implementation{
    struct Device : DeviceT<Device>{
        Device(hstring const& cname, hstring const& dname);
        hstring ClassName();
        hstring DeviceName();
    protected:
        hstring class_name;
        hstring device_name;
    };
}

namespace winrt::gui::Models::factory_implementation{
    struct Device : DeviceT<Device, implementation::Device>{
    };
}
