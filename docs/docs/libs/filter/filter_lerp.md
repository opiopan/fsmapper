---
sidebar_position: 4
---

# filter.lerp()
```lua
filter.lerp(action, val_mappings)
```
This function create a native-action to modify the characteristics curve of a device's analog axis.<br/>
You can specify a mapping rule that maps numeric Event Values to other numeric values by specifying pairs of input and output values.
For values other than the specified ones, the mapping is determined through linear interpolation.

## Prameters
|Parameter|Type|Description|
|-|-|-|
|`action`|[Action](/guide/event-action-mapping#action)|An action that receives the mapped value through linear interpolation as the Event Value.
|`val_mappings`|table|Specifies the mapping rules as an array table containing sub-tables, such as `{100, 500}`, which represent the relationship between input values and corresponding output values.


## Return Values
This function returns a native-action.
