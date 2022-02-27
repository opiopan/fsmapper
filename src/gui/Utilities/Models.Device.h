#pragma once
#include "Models.Device.g.h"

namespace winrt::gui::Models::implementation{
    struct Device : DeviceT<Device>
    {
        Device() = default;

        hstring className();
        hstring deviceName();
    };
}

namespace winrt::gui::Models::factory_implementation
{
    struct Device : DeviceT<Device, implementation::Device>
    {
    };
}
