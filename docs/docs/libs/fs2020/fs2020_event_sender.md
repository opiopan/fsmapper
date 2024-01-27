---
sidebar_position: 2
---

# fs2020.event_sender()
```lua
fs2020.event_sender(event_name[, param1[, param2[, param3[, param4[, param5]]]])
```
This function creates a native-action to send a SimConnect client [Event ID](https://docs.flightsimulator.com/html/Programming_Tools/Event_IDs/Event_IDs.htm) to the Microsoft Flight Simulator.

:::warning Note
This function is recommended to be replaced with [`fs2020.mfwasm.rpn_executer()`](/libs/fs2020/fs2020_mfwasm_rpn_executer). It provides greater flexibility for specifying operations, and allows for operations that cannot be achieved with [Event ID](https://docs.flightsimulator.com/html/Programming_Tools/Event_IDs/Event_IDs.htm).
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
This function returns a native-action.