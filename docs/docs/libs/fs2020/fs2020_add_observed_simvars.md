---
sidebar_position: 3
---

# fs2020.add_observed_simvars()
```lua
fs2020.add_observed_simvars(def_array)
```
This function registers [SimVars](https://docs.flightsimulator.com/html/Programming_Tools/SimVars/Simulation_Variables.htm) for observing.
It triggers the specified [events](/guide/event-action-mapping#event) in response to changes in the values of the registered [SimVars](https://docs.flightsimulator.com/html/Programming_Tools/SimVars/Simulation_Variables.htm).
In this case, the [Event Value](/guide/event-action-mapping#event) corresponds to the [SimVar](https://docs.flightsimulator.com/html/Programming_Tools/SimVars/Simulation_Variables.htm)'s value.

:::warning Note
This function is recommended to be replaced with [`fs2020.mfwasm.add_observed_data()`](/libs/fs2020/fs2020_mfwasm_add_observed_data). It becomes possible to access the internal states of MSFS that are not accessible through [SimVars](https://docs.flightsimulator.com/html/Programming_Tools/SimVars/Simulation_Variables.htm), allowing for a more flexible data expression.
:::

## Prameters
|Parameter|Type|Description|
|-|-|-|
|`def_array`|table|This parameter specifies the SimVars to be observed as an array of  [Observed SimVar Definition](#observed-simvar-definition).


### Observed SimVar Definition
|Key|Type|Description|
|-|-|-|
|`name`|string|Name of [SimVar](https://docs.flightsimulator.com/html/Programming_Tools/SimVars/Simulation_Variables.htm) for observing.<br/>This parameter is required.
|`unit`|string|[Unit](https://docs.flightsimulator.com/html/Programming_Tools/SimVars/Simulation_Variable_Units.htm) of [SimVar](https://docs.flightsimulator.com/html/Programming_Tools/SimVars/Simulation_Variables.htm) for observing.<br/>This parameter is required.
|`event`|numeric|[Event ID](/guide/event-action-mapping#event) that occurs when the observed [SimVar](https://docs.flightsimulator.com/html/Programming_Tools/SimVars/Simulation_Variables.htm) changes.<br/>This parameter is required.
|`epsilon`|numeric|If the change in the observed [SimVar](https://docs.flightsimulator.com/html/Programming_Tools/SimVars/Simulation_Variables.htm) does not exceed this value, no event will be triggered.<br/>Choosing an appropriate value helps prevent unnecessary event generation, reducing system load.<br/>This parameter is optional. The default is `0`.


## Return Values
This function doesn't return any value.