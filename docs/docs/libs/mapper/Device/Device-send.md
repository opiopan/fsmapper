---
sidebar_position: 6
---

# Device:send()
```lua
Device:send(unit_id, value)
```
This method sends a value to output unit.


## Parameters
|Parameter|Type|Description|
|-|-|-|
|`unit_id`|number|Output device unit ID|
|`value`|number|Value to be sent|


## Return Values
This method doesn't return any value.

## See Also
- [Device Handling](/guide/device)
- [Output Unit IDs Table](/guide/device/#output-unit-ids-table)
- [`Device.upstream_ids`](/libs/mapper/Device/Device_upstream_ids)
- [`Device:get_upstream_ids()`](/libs/mapper/Device/Device-get_upstream_ids)
- [`Device:sender()`](/libs/mapper/Device/Device-sender)
