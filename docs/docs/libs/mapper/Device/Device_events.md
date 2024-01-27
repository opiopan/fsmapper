---
sidebar_position: 1
---

# Device.events
```lua
Device.events
```
A table holding event IDs for each input unit.<br/>
This property is not able to be update.


## Type
Assosiative array table.

The structure of this table is a two-level associative array. The first level uses the device unit name as the key, and its value is a second-level associative array table where the keys are event names determined by the event modifier. The values in the second-level associative array table represent the event IDs.
