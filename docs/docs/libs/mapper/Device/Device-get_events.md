---
sidebar_position: 3
---

# Device:get_events()
```lua
Device:get_events()
```
This method returns a table holding event IDs for each input unit.


## Return Values
This method returns a assosiative array table.<br/>
The structure of this table is a two-level associative array.
The first level uses the device unit name as the key, and its value is a second-level associative array table where the keys are event names determined by the event modifier.
The values in the second-level associative array table represent the event IDs.
