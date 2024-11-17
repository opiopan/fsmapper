---
sidebar_position: 23
---

# mapper.virtual_joystick()
```lua
mapper.virtual_joystick(devid)
```
This funciton opens the vJoy device and create a [`vJoy`](/libs/mapper/vJoy) object that functions as a feeder.

:::warning Note
If you use this function, you need to have [**vJoy**](https://sourceforge.net/projects/vjoystick/) installed.
:::

## Parameters
|Parameter|Type|Description|
|-|-|-|
|`devid`|number|Device ID of the vJoy device.


## Return Values
This function returns a [`vJoy`](/libs/mapper/vJoy) object.

## See Also
- [Joystick emulation](/guide/input_emulation/#joystick-emulation)
- [`vJoy`](/libs/mapper/vJoy)