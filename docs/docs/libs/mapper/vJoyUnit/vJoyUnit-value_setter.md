---
sidebar_position: 2
---

# vJoyUnit:value_setter()
```lua
vJoyUnit:value_setter([value])
```
This method creates a native-actionn to set a valuev for the operable unit of the vJoy device.<br/>
The values that can be configured for each unit are as follows.

|Unit Type|Values|
|---------|------|
|Axis|-50000 to 50000
|Button|1 when the button is pressed, 0 when released
|POV|The angle in degrees.

## Prameters
|Parameter|Type|Description|
|-|-|-|
|`value`|numeric|Value to be set for the unit.<br/>If this parameter is not specified, the event value is set to the unit.

## Return Values
This method returns a native-action.