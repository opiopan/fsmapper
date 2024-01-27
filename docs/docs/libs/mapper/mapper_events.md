---
sidebar_position: 3
---

# mapper.events
```lua
mapper.events
```
This varialbe referes to a table held system event IDs


## Type
Associative array table with the following keys.

|Key|Type|Description|
|---|----|-----------|
|`change_aircraft`|numeric|The event ID for the event that occurs when Flight Simulator starts or stops, or when the selected aircraft changes. The **Event Value** is a associative array table where the `host` key holds the running Flight Simulator software and the `aircraft` key contains the selected aircraft.<br/>Currently, fsmapper only supports Microsoft Flight Simurator 2020, so the `host` value is either `'fs2020'` or `nil`.
