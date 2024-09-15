---
sidebar_position: 2
---

# dcs.clickable_action_performer()
```lua
dcs.clickable_action_performer(device_id, command[, value, ...])
```
This function creates a native-action to set the position of the clickable control specified by `device_id` and `command` in the cockpit to the specified `value` in DCS world.
If multiple `value`s are specified, they will be set to the position of the clickable control in the order they were provided.
If `value` is not specified, the value of the event passed to the native action will be used.

The native-action created by function applies the same effect to the aircraft in flight as the ‘**X: COCKPIT PERFORM CLICKABLE ACTION**’ in the DCS World Mission Editor. Each argument provided to the function corresponds to the same parameters as in ‘**X: COCKPIT PERFORM CLICKABLE ACTION**'. For more details, refer to the [DCS User Manual](https://www.digitalcombatsimulator.com/en/downloads/documentation/dcs-user_manual_en/).


## Parameters
|Parameter|Type|Description|
|-|-|-|
|`device_id`|number|ID value of the cockpit device manipulated by the required control. Device ID’s are described in `DCS\Mods\aircrafts\ Aircraft Name\Cockpit\Scripts\devices.lua` .
|`command`|number|The command value for the required control. Command values are described in `DCS\Mods\aircrafts\Aircraft Name\Cockpit\Scripts\clickabledata.lua`.
|`value`|number|The value representing the control position in the cockpit. If this parameter is not specified, the value of the event passed to the native action will be used.


## Return Values
This function returns a native-action.

## See Also
- [Interaction with DCS](/guide/dcs)
- [Setting position of cockpit controls](/guide/dcs#setting-position-of-cockpit-controls)
- [`dcs.perform_clickable_action()`](/libs/dcs/dcs_perform_clickable_action)
