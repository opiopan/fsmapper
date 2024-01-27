---
sidebar_position: 2
---

# Device.upstream_ids
```lua
Device.upstream_ids
```
A table holding unit IDs for each output unit.<br/>
This property is not able to be update.


## Type
Assosiative array table.


This associative array table uses the device unit name as the key, and the value contains the device unit ID.
The device unit ID from this table is used as an argument for methods like [`Device:send()`](/libs/mapper/Device/Device-send) or [`Device:sender()`](/libs/mapper/Device/Device-sender).