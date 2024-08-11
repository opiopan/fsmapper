---
sidebar_position: 3
---

# mapper.events
```lua
mapper.events
```
This variable refers to a table held system event IDs.


## Type
Associative array table with the following keys.

|Key|Type|Description|
|---|----|-----------|
|`change_aircraft`|numeric|The event ID for the event that occurs when fsmapper connects to the flight simulator or the connection is closed, or when the selected aircraft changes.<br/>The event value for this event is a associative array table that holds the simulator type and aircraft name. For details, refer to the [**SimAircraft Table**](#simaircraft-table).

## Event Value Types

### SimAircraft Table
The event value for `change_aircraft` event is an associative array table with the following keys.

|Key|Type|Description|
|---|----|-----------|
|`sim_type`|string|The type of flight simulator that fsmapper is connected to. If the connection to the flight simulator is lost, this will be set to `nil`. You can interact with the flight simulator using the same functions as long as this value remains the same.<br/>One of the following strings will be set.<br/><br/><ul><li>`msfs` : Microsoft Flight Simulator or SimConnect-compatible simulator</li><li>`dcs` : Eagle Dynamics DCS World</li></ul>
|`sim_detail`|string|More detailed information about the connected flight simulator than `sim_type`. One of the following strings will be set.<br/><br/><ul><li>`fsx`: Microsoft Flight Simulator X</li><li>`fs2020`: Microsoft Flight Simulator 2020</li><li>`fs2024`: Microsoft Flight Simulator 2024</li><li>`SimConnect` : Other SimConnect-compatible simulators not listed above</li><li>`dcs` : Eagle Dynamics DCS World</li></ul>
|`aircraft`|string|The name of the aircraft the player is piloting.
|`host`|string|Same as `sim_detail`. This field is for compatibility with previous versions of fsmapper.


## See Also
- [Event Action Mapping](/guide/event-action-mapping)