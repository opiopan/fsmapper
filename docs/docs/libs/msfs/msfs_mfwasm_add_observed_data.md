---
sidebar_position: 7
---

# msfs.mfwasm.add_observed_data()
```lua
msfs.mfwasm.add_observed_data(def_array)
```
This function registers MSFS internal data as expressed in [RPN](https://docs.flightsimulator.com/html/Additional_Information/Reverse_Polish_Notation.htm) for observing.
It triggers the specified [events](/guide/event-action-mapping#event) in response to changes in the values of the registered data expressed in [RPN](https://docs.flightsimulator.com/html/Additional_Information/Reverse_Polish_Notation.htm).
In this case, the [Event Value](/guide/event-action-mapping#event) corresponds to the result of evaluating the [RPN](https://docs.flightsimulator.com/html/Additional_Information/Reverse_Polish_Notation.htm).

:::warning Note
If you use this function, you need to have [**MobiFlight WASM Module**](https://github.com/MobiFlight/MobiFlight-WASM-Module) installed.
:::

## Parameters
|Parameter|Type|Description|
|-|-|-|
|`def_array`|table|This parameter specifies the data to be observed as an array of  [Observed Data Definition](#observed-data-definition).

### Observed Data Definition
|Key|Type|Description|
|---|----|-----------|
|`event`|numeric|[Event ID](/guide/event-action-mapping#event) for the event triggered when the observed data changes
|`rpn`|string|[RPN](https://docs.flightsimulator.com/html/Additional_Information/Reverse_Polish_Notation.htm) expression representing the observed data
|`initial_value`|numeric|The initial value of the observed data.<br/>fsmapper trigers an event with the initial value as the [Event Value](/guide/event-action-mapping#event).<br/>This parameter is optional. The default is `0`.
|`epsilon`|numeric|If the change in the observed data does not exceed this value, no event will be triggered. Choosing an appropriate value helps prevent unnecessary event generation, reducing system load.<br/>This parameter is optional. The default is `0`


## Return Values
This function doesn't return any value.

## See Also
- [Interaction with MSFS](/guide/msfs)
- [Retrieving MSFS's Internal State](/guide/msfs#retrieving-msfss-internal-state)
- [`msfs.mfwasm.clear_observed_data()`](/libs/msfs/msfs_mfwasm_clear_observed_data)
