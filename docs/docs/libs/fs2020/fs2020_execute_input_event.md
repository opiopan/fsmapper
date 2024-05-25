---
sidebar_position: 4.1
---

# fs2020.execute_input_event()
```lua
fs2020.execute_input_event(input_event, value)
```
This function issues an [Input Event](https://docs.flightsimulator.com/html/Content_Configuration/Models/ModelBehaviors/Input_Event_Definitions.htm) into the Microsoft Flight Simulator.

Note that the preset executed by this function is the `_Set` preset within the Input Event definition specified by `input_event`.
Due to limitations of the SimConnect specifications, it is not possible to execute any presets other than `_Set`.
Additionally, note that the `input_event specified` should be the Input Event ID without the preset name.

## Parameters
|Parameter|Type|Description|
|-|-|-|
|`input_event`|string|[Input Event ID](https://docs.flightsimulator.com/html/Content_Configuration/Models/ModelBehaviors/General_Template_Definitions.htm?#InputEvent)
|`value`|numeric or string|Value to set for [Input Event](https://docs.flightsimulator.com/html/Content_Configuration/Models/ModelBehaviors/Input_Event_Definitions.htm)


## Return Values
This function doesn't return any value.

## See Also
- [Interaction with MSFS](/guide/msfs)
- [Issueing an Input Event](/guide/msfs#issueing-an-input-event)
- [`fs2020.input_event_executer()`](/libs/fs2020/fs2020_input_event_executer)
