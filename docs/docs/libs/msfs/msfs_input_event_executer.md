---
sidebar_position: 4.2
---

# msfs.input_event_executer()
```lua
msfs.input_event_executer(input_event[, value])
```
This function creates a native-action to issue an [Input Event](https://docs.flightsimulator.com/html/Content_Configuration/Models/ModelBehaviors/Input_Event_Definitions.htm).

Note that the preset executed by this function is the `_Set` preset within the Input Event definition specified by `input_event`.
Due to limitations of the SimConnect specifications, it is not possible to execute any presets other than `_Set`.
Additionally, note that the `input_event specified` should be the Input Event ID without the preset name.


## Parameters
|Parameter|Type|Description|
|-|-|-|
|`input_event`|string|[Input Event ID](https://docs.flightsimulator.com/html/Content_Configuration/Models/ModelBehaviors/General_Template_Definitions.htm?#InputEvent)
|`value`|number or string|Value to set for [Input Event](https://docs.flightsimulator.com/html/Content_Configuration/Models/ModelBehaviors/Input_Event_Definitions.htm)<br/>If this parameter is omitted, the value of event passed to the native action will be used.


## Return Values
This function returns a native-action.

## See Also
- [Interaction with MSFS](/guide/msfs)
- [Issueing an Input Event](/guide/msfs#issueing-an-input-event)
- [`msfs.execute_input_event()`](/libs/msfs/msfs_execute_input_event)
