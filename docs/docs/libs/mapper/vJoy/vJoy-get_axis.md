---
sidebar_position: 1
---

# vJoy:get_axis()
```lua
vJoy:get_axis(axis)
```
This method is used to get a [`vJoyUnit`](/libs/mapper/vJoyUnit) object corresponding to an analog axis.


## Prameters
|Parameter|Type|Description|
|-|-|-|
|`axis`|string|Name of the analog axis.<br/>It specifies one of the following names: `'x'`, `'y'`, `'z'`, `'rx'`, `'ry'`, `'rz'`, `'slider1'`, `'slider2'`.


## Return Values
This method returns a [`vJoyUnit`](/libs/mapper/vJoyUnit) object.