---
sidebar_position: 7
---

# Device:sender()
```lua
Device:sender(unit_id[, value])
```
This method creates a [native-action](/guide/event-action-mapping#action) to send a value to output unit.


## Parameters
|Parameter|Type|Description|
|-|-|-|
|`unit_id`|number|Output device unit ID.
|`value`|number|Value to be sent.<br/>If this parameter is not specified, the event value is sent to the output device unit.

## Return Values
This method returns a [native-action](/guide/event-action-mapping#action).

## See Also
- [Device Handling](/guide/device)
- [Output Unit IDs Table](/guide/device/#output-unit-ids-table)
- [`Device.upstream_ids`](/libs/mapper/Device/Device_upstream_ids)
- [`Device:get_upstream_ids()`](/libs/mapper/Device/Device-get_upstream_ids)
- [`Device:send()`](/libs/mapper/Device/Device-send)
