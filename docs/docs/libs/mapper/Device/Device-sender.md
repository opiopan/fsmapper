---
sidebar_position: 7
---

# Device:sender()
```lua
Device:sender(unit_id[, value])
```
This method creates a [native-action](/guide/event-action-mapping#action) to send a value to output unit.


## Prameters
|Parameter|Type|Description|
|-|-|-|
|`unit_id`|numeric|Output device unit ID.
|`value`|numeric|Value to be sent.<br/>If this parameter is not specified, the event value is sent to the output device unit.

## Return Values
This method returns a [native-action](/guide/event-action-mapping#action).