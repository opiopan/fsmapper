---
sidebar_position: 1
---

# vJoyUnit:set_value()
```lua
vJoyUnit:set_value(value)
```
This method sets a value for the operable unit of the vJoy device.<br/>
The values that can be configured for each unit are as follows.

|Unit Type|Values|
|---------|------|
|Axis|-50000 to 50000
|Button|1 when the button is pressed, 0 when released
|POV|The angle in degrees.


## Prameters
|Parameter|Type|Description|
|-|-|-|
|`value`|numeric|Value to be set for the unit.


## Return Values
This method doesn't return any value.