---
sidebar_position: 4
---

# Interaction with MSFS
This section explains the mechanism prepared to control Microsoft Flight Simulator (MSFS) from fsmapper.

## Basic Concept
The functionality prepared by fsmapper for integration with MSFS serves to achieve two main objectives.

- Reflect the operations of external devices or similar on the various controls within the cockpit of aircraft in MSFS.
- Mirror the status of cockpit instruments onto external devices or similar.

These actions are similar to what aircraft mod creators for MSFS do when defining behaviors for the cockpit's 3D models:

- Reflecting mouse or controller operations onto the simulator's internal state or cockpit animations.
- Reflecting the simulator's internal state onto gauge needle angles or lamp status.

As mentioned in the [**Microsoft Flight Simulator SDK** documentation](https://docs.flightsimulator.com/html/Content_Configuration/Models/ModelBehaviors/Model_Behaviors.htm), in MSFS, these actions are defined in the XML `<Behaviors>` element, linking components to their internal states using [**Reverse Polish Notation** (RPN)](https://docs.flightsimulator.com/html/Additional_Information/Reverse_Polish_Notation.htm).<br/>
fsmapper leverages the [**MobiFlight WASM Module**](https://github.com/MobiFlight/MobiFlight-WASM-Module) to execute [**RPN**](https://docs.flightsimulator.com/html/Additional_Information/Reverse_Polish_Notation.htm) through [`execute_calculator_code()`](https://docs.flightsimulator.com/html/Programming_Tools/WASM/Gauge_API/execute_calculator_code.htm) in the [**Gauge API**](https://docs.flightsimulator.com/html/Programming_Tools/WASM/Gauge_API/Gauge_API.htm). This enables Lua scripts to describe what aircraft mods are performing internally within MSFS.

Moving forward, the explanation on altering the internal state of MSFS from Lua scripts and retrieving the internal state of MSFS using Lua Scripts will be provided.

:::warning Deprecated Functions
When the initial version of fsmapper was launched in 2021, interaction with the internal state of MSFS was done through [**SimVars**](https://docs.flightsimulator.com/html/Programming_Tools/SimVars/Simulation_Variables.htm) and [**Event IDs**](https://docs.flightsimulator.com/html/Programming_Tools/Event_IDs/Event_IDs.htm). Although functions for these interactions are still retained for compatibility with legacy scripts, it is not recommended to use them. Interactions via [**RPN**](https://docs.flightsimulator.com/html/Additional_Information/Reverse_Polish_Notation.htm) are much more flexible and cover a broader range.<br/>

This section does not provide explanations for these deprecated functions.
:::

## Changing MSFS's Internal State
fsmapper provides the function [`msfs.mfwasm.execute_rpn()`](/libs/msfs/msfs_mfwasm_execute_rpn) to run [**RPN**](https://docs.flightsimulator.com/html/Additional_Information/Reverse_Polish_Notation.htm) within MSFS and [`msfs.mfwasm.rpn_executer()`](/libs/msfs/msfs_mfwasm_rpn_executer) to generate a [**native-action**](/guide/event-action-mapping#action) for executing [**RPN**](https://docs.flightsimulator.com/html/Additional_Information/Reverse_Polish_Notation.htm).

Here's a code snippet that increases the VOR1 OBI setting by 10 degrees in many aircraft.

```lua
mapper.mfwasm.execute_rpn('(>K:VOR1_OBI_FAST_INC)')
```

:::warning B variables issue
Although [**RPN**](https://docs.flightsimulator.com/html/Additional_Information/Reverse_Polish_Notation.htm) itself is very powerful in terms of expressive capability, 
due to the limitations of [`execute_calculator_code()`](https://docs.flightsimulator.com/html/Programming_Tools/WASM/Gauge_API/execute_calculator_code.htm) in the [**Gauge API**](https://docs.flightsimulator.com/html/Programming_Tools/WASM/Gauge_API/Gauge_API.htm), 
it cannot access [**B variables**](https://docs.flightsimulator.com/html/Additional_Information/Reverse_Polish_Notation.htm?#Types), meaning it cannot get or set values from an [**Input Event**](https://docs.flightsimulator.com/html/Content_Configuration/Models/ModelBehaviors/Input_Event_Definitions.htm). fsmapper provides separate functions for setting value of an Input Event (i.e., issuing an Input Event). 
For details on how to issue an Input Event, refer to [**Issuing an Input Event**](#issuing-an-input-event).
:::

## Retrieving MSFS's Internal State
fsmapper provides a mechanism to monitor the evaluated values of [**RPN**](https://docs.flightsimulator.com/html/Additional_Information/Reverse_Polish_Notation.htm) and notify any changes in values as [**Event**](/guide/event-action-mapping#event)s for referencing the internal state of MSFS. 
You can register the monitored RPN expressions by specifying an array of tables following the definitions below in [`msfs.mfwasm.add_observed_data()`](/libs/msfs/msfs_mfwasm_add_observed_data).

|Key|Type|Description|
|---|----|-----------|
|`event`|number|[**Event ID**](/guide/event-action-mapping#event) for the event triggered when the monitored data changes
|`rpn`|string|[**RPN**](https://docs.flightsimulator.com/html/Additional_Information/Reverse_Polish_Notation.htm) expression representing the monitored data
|`initial_value`|number|The initial value of the monitored data.<br/>fsmapper triggers an event with the initial value as the [**Event Value**](/guide/event-action-mapping#event).<br/>This parameter is optional. The default is `0`.
|`epsilon`|number|If the change in the monitored data does not exceed this value, no event will be triggered. Choosing an appropriate value helps prevent unnecessary event generation, reducing system load.<br/>This parameter is optional. The default is `0`

The following code snippet enables notification of changes in the aircraft's heading as the HEADING event, and the corresponding action can display the heading in the message console.

```lua
local hdg_event = mapper.register_event('HEADING')
msfs.mfwasm.add_observed_data{
    {
        event = hdg_event,
        rpn='(A:HEADING INDICATOR, Degrees)',
        epsilon=0.5
    }
}

mapper.add_primary_mappings{
    {
        event = hdg_event,
        action = function (event_id, value)
            mapper.print('Heading: ' .. value)
        end
    }
}
```

## Issuing an Input Event
[**Input Events**](https://docs.flightsimulator.com/html/Content_Configuration/Models/ModelBehaviors/Input_Event_Definitions.htm) is a system in MSFS used to define the behavior of cockpit interactable objects when accessed with a mouse. 
While it is possible to implement cockpit behavior without defining Input Events, in most cases, Input Events are defined for each type of interaction with objects such as switches and knobs.

Therefore, using [**Input Events**](https://docs.flightsimulator.com/html/Content_Configuration/Models/ModelBehaviors/Input_Event_Definitions.htm) to link the operations of physical devices or virtual cockpit controls with the operations in the MSFS cockpit is a logical strategy.

However, due to the limitations of [`execute_calculator_code()`](https://docs.flightsimulator.com/html/Programming_Tools/WASM/Gauge_API/execute_calculator_code.htm) in the [**Gauge API**](https://docs.flightsimulator.com/html/Programming_Tools/WASM/Gauge_API/Gauge_API.htm), [**RPN scripts**](https://docs.flightsimulator.com/html/Additional_Information/Reverse_Polish_Notation.htm) that issue [**Input Events**](https://docs.flightsimulator.com/html/Content_Configuration/Models/ModelBehaviors/Input_Event_Definitions.htm) do not function correctly.
To address this, fsmapper provides separate functions for issuing an Input Event. 
Similar to the functions for executing RPN, there is [`msfs.execute_input_event()`](/libs/msfs/msfs_execute_input_event) for issuing an Input Event and [`msfs.input_event_executer()`](/libs/msfs/msfs_input_event_executer) which returns the Native Event for issuing an Input Event.

```lua
-- definition of event-action mappings to issue Input Events
local mappings = {
    {
        event = joystick.button1.down,
        action = function () msfs.execute_input_event('LIGHTING_LANDING_0', 1) end,
    },
    {
        event = joystick.button2.down,
        action = msfs.input_event_executer('LIGHTING_LANDING_0', 0),
    },
}
```