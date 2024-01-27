---
sidebar_position: 1
---

# fs2020.send_event()
```lua
fs2020.send_event(event_name[, param1[, param2[, param3[, param4[, param5]]]])
```
This function send a [Event ID](https://docs.flightsimulator.com/html/Programming_Tools/Event_IDs/Event_IDs.htm) to the Microsoft Flight Simulator.

:::warning Note
This function is recommended to be replaced with [`fs2020.mfwasm.execute_rpn()`](/libs/fs2020/fs2020_mfwasm_execute_rpn). It provides greater flexibility for specifying operations, and allows for operations that cannot be achieved with [Event ID](https://docs.flightsimulator.com/html/Programming_Tools/Event_IDs/Event_IDs.htm).
:::

## Prameters
|Parameter|Type|Description|
|-|-|-|
|`event_name`|string|[Event ID](https://docs.flightsimulator.com/html/Programming_Tools/Event_IDs/Event_IDs.htm)
|`param1`|numeric|1st parameter of the event.
|`param2`|numeric|2nd parameter of the event.
|`param3`|numeric|3rd parameter of the event.
|`param4`|numeric|4th parameter of the event.
|`param5`|numeric|5th parameter of the event.


## Return Values
This function doesn't return any value.