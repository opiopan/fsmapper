---
sidebar_position: 7
---

# dcs.add_observed_data()
```lua
dcs.add_observed_data(def_array)
```
This function register the definitions of cockpit data to be observed specified as [Observed Data Definition](#observed-data-definition) table.
It triggers the event which has specified [Event ID](/guide/event-action-mapping#event) in response to changes in the values of the registered cockpit data. The [Event Value](/guide/event-action-mapping#event) is exactly the value of the observed cockpit data.

There are three ways to specify the observed cockpit data.
- **Cockpit Control Position**<br/>
    By specifying the `argument_number` parameter in the [Observed Data Definition](#observed-data-definition), you can observe elements such as switch positions, button press states, and indicator lamp statuses. The observation targets you can specify are the same as those covered by the ‘**X: COCKPIT ARGUMENT IN RANGE**’ trigger in the DCS Mission Editor.

- **Indication Text**<br/>
    By specifying the `indicator_id` parameter in the [Observed Data Definition](#observed-data-definition), you can observe the display content of indicators such as the IFEI in the F/A-18C or the DED in the F-16C. The indicators you can observe are the same as those covered by the ‘**X: COCKPIT INDICATION TEXT IS EQUAL TO**’ trigger in the DCS Mission Editor.

- **Lua Chunk value**<br/>
    When you specify a Lua chunk as a string in the `chunk` parameter of the [Observed Data Definition](#observed-data-definition), the return value of the chunk is observed. This method allows for complex behaviors, such as creating an [Event Value](/guide/event-action-mapping#event) that combines the states of multiple controls.
    Unless you specify a `filter` to change the type of the value, the return value of the chunk must be either a string or a number.


:::info Note
To find the corresponding values for the `argument_number` or `indicator_id` parameters for cockpit instruments of each aircraft module, 
refer to the '**X: COCKPIT ARGUMENT IN RANGE**' trigger and the '**X: COCKPIT INDICATION TEXT IS EQUAL TO**' trigger sections in the [DCS User Manual](https://www.digitalcombatsimulator.com/en/downloads/documentation/dcs-user_manual_en/).
:::

## Parameters
|Parameter|Type|Description|
|-|-|-|
|`def_array`|table|This parameter specifies the data to be observed as an array of [Observed Data Definition](#observed-data-definition).


### Observed Data Definition
|Key|Type|Description|
|-|-|-|
|`event`|number|[Event ID](/guide/event-action-mapping#event) for the event triggered when the observed data changes
|`argument_number`|number|The ID number of the cockpit argument to be observed. You must specify only one of the following parameters, `argument_number`, `indicator_id`, or `chunk`.
|`indicator_id`|number|The ID value of the cockpit indicator to be observed. You must specify only one of the following parameters, `argument_number`, `indicator_id`, or `chunk`.
|`chunk`|string|A string representing the Lua chunk that returns the value to be observed.​ You must specify only one of the following parameters, `argument_number`, `indicator_id`, or `chunk`.
|`filter`|table<br/>string|This parameter is used to specify the conditions that trigger an event, either by limiting it to when the observed data matches a specific value or by transforming the value.<br/>If an array table is provided, the event is triggered when the observed value changes to one of the values in the table.<br/>If a string representing a Lua chunk is specified, the chunk is called every frame in DCS World, with the observed value passed as an argument. The return value of the chunk becomes the [Event Value](/guide/event-action-mapping#event). However, if the chunk does not return a value, or if the return value’s type is not a string or number, the event will not be triggered.<br/>This parameter is optional.
|`epsilon`|number|If the change in the value of the observed data after passing through the filter does not exceed this value, no event will be triggered. Choosing an appropriate value helps prevent unnecessary event generation, reducing system load. This specification is ignored if the value type of the observed data is not a number.<br/>This parameter is optional. The default is 0

:::warning note
While fsmapper uses [Lua 5.4](https://www.lua.org/manual/5.4/manual.html), DCS World is built with [Lua 5.1](https://www.lua.org/manual/5.1/manual.html). The string specified for `chunk` parameter and `filter` parameter must comply with [Lua 5.1](https://www.lua.org/manual/5.1/manual.html) specification.
:::


## Return Values
This function doesn't return any value.

## See Also
- [Interaction with DCS](/guide/dcs)
- [Retrieving DCS's Internal State](/guide/dcs#retrieving-dcss-internal-state)
- [`dcs.clear_observed_data()`](/libs/dcs/dcs_clear_observed_data)
