---
sidebar_position: 4
---

# Device:get_upstream_ids()
```lua
Device:get_upstream_ids()
```
This method returns a table holding unit IDs for each output unit.


## Return Values
This method returns a assosiative array table.<br/>
This associative array table uses the device unit name as the key, and the value contains the device unit ID.
The device unit ID from this table is used as an argument for methods like [`Device:send()`](/libs/mapper/Device/Device-send) or [`Device:sender()`](/libs/mapper/Device/Device-sender).