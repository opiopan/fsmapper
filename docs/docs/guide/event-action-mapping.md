---
sidebar_position: 2
---

# Event-Action Mapping
The core of fsmapper's configuration lies in defining actions that are triggered in response to generated events.
This section explains how to register these event-action mappings within fsmapper and the crucial concepts associated with it.

## Event
Events are identified by unique 64-bit integers assigned to distinguish their types, referred to as **Event ID**s.
Additionally, each Event ID is given a name in string format for easier user recognition; however, there's no constraint for these names to be unique.
While fsmapper sometimes automatically assigns Event IDs, users can also explicitly designate Event IDs within the system through Lua scripts. To assign Event IDs via scripts, the [`mapper.register_event()`](/libs/mapper/mapper_register_event) function is utilized.
```lua
local my_event = mapper.register_event('My Event')
```

When events occur from input devices like flight sticks or from event sources like MSFS, they get stored in fsmapper's event queue.
fsmapper calls corresponding actions in the order these events are stored in the queue.

Events stored in the event queue not only retain their Event IDs but also hold values specified by the event source, which are then passed to actions.
These values are referred to as **Event Value**s.
The type of Event Values varies depending on the type of event. For example, an event triggered by manipulation of the throttle device sets a numerical value representing the throttle position as its Event Value. In contrast, events triggered when switching aircraft within the flight simulator have an Event Value in a Lua table containing simulator software specifics and the name of the aircraft.

Apart from events that occur asynchronously from each event source, it's also possible to explicitly generate events from Lua scripts. For this purpose, [`mapper.raise_event()`](/libs/mapper/mapper_raise_event) has been made available.
```lua
mapper.raise_event(my_event, 'this is a associated value for a event')
```


## Action
Actions are commonly defined as Lua functions, but it's also possible to utilize special objects provided by fsmapper known as **native-action**s.

When using Lua functions as actions, the **Event ID** is passed as the first argument and the **Event Value** as the second.
```lua title="Action Function Definition"
function ACTION(event_id, event_value)
```

Native-actions are unique objects that behave as actions compiled into native code. When fsmapper calls a native-action, it can do so directly without going through the Lua interpreter, reducing system load. Additionally, when events and actions are displayed in the message console, Native-actions provide clearer information than Lua function.

Here's an example defining the same action using both a Lua function and a native-action.
```lua
-- Note that variable axis is binded with a vJoyUnit object
local action_lua = function (event_id, event_value)
    axis:set_value(event_value)
end

local action_native = axis:value_setter()
```

Below is a list of functions or methods that provide native-action
- [`Device:sender()`](/libs/mapper/Device/Device-sender)
- [`Canvas:value_setter()`](/libs/mapper/Canvas/Canvas-value_setter)
- [`vJoyUnit:value_setter()`](/libs/mapper/vJoyUnit/vJoyUnit-value_setter)
- [`Keystroke:synthesize()`](/libs/mapper/Keystroke/Keystroke-synthesizer)
- [filter library](/libs/filter)
- [`fs2020.event_sender()`](/libs/fs2020/fs2020_event_sender)
- [`fs2020.mfwasm.rpn_executer()`](/libs/fs2020/fs2020_mfwasm_rpn_executer)

## Event-Action mapping definition
When registering the correspondence between events and actions in fsmapper, 
it's instructed using a Lua table object known as the **Event-Action Mapping definition**.
This mapping is an associative array that holds the Event ID under the key `event` and the action under the key `action`.

To actually register the correspondence between events and actions in fsmapper, you pass an array of Event-Action Mapping definition tables.
```lua
mapper.set_primary_mappings{
    {event=evid1, action=action_func1},
    {event=evid2, action=action_func2},
    {event=evid3, action=action_func3},
    {event=evid4, action=action_func4},
}
```

## Cascading Event-Action mappings
Event-Action mappings are managed across multiple hierarchies. 
It allows switching mapping definitions dynamically or overriding behaviors with definitions from higher-priority hierarchies.

Here are the levels where Event-Action mappings can be registered and how actions are selected in case multiple actions are registered for the same Event ID with different priorities.

|Level | Priority | Remarks|
|------|----------|--------|
|Primary Mappings|4||
|Secondary Mappings|3||
|Mappings associated with a [Viewport](/guide/virtual_instrument_panel#components-for-virtual-instrument-panel)|2|Enabled only when the viewport is displayed|
|Mappings associated with a [View](/guide/virtual_instrument_panel#components-for-virtual-instrument-panel)|1|Enabled only when the viewport is displayed and the view is the current view in the viewport|

Here are the functions to configure and modify Event-Action mappings.

|Level | Functions to change mappings| 
|------|----------|
|Primary Mappings|[`mapper.set_primary_mappings()`](/libs/mapper/mapper_set_primary_mappings)<br/>[`mapper.add_primary_mappings()`](/libs/mapper/mapper_add_primary_mappings)
|Secondary Mappings|[`mapper.set_secondary_mappings()`](/libs/mapper/mapper_set_secondary_mappings)<br/>[`mapper.add_secondary_mappings()`](/libs/mapper/mapper_add_secondary_mappings)
|Mappings associated with a [Viewport](/guide/virtual_instrument_panel#components-for-virtual-instrument-panel)|[`Viewport:set_mappings()`](/libs/mapper/Viewport/Viewport-set_mappings)<br/>[`Viewport:add_mappings()`](/libs/mapper/Viewport/Viewport-add_mappings)
|Mappings associated with a [View](/guide/virtual_instrument_panel#components-for-virtual-instrument-panel)|[`Viewport:register_view()`](/libs/mapper/Viewport/Viewport-register_view)

:::tip
Looking at the above functions, you might have wondered, "There's set and add, but no clear?" 
To clear an **Event-Action mappings**, setting an empty array achieves the same result, so a separate 'clear' function wasn't provided.<br/>
If you wish to clear it, do as followings.

```lua
mapper.set_primary_mappings({})
```
:::

## Filter
The [**filter library**](/libs/filter/) enables processing events, performing conditional branching, and similar operations through cascaded connections of native-actions.
By combining the native-actions provided by the [**filter library**](http://localhost:3000/fsmapper-docs/libs/filter/) with other native-actions, complex operations can be efficiently executed as native code without the intervention of the Lua interpreter.

Below is an example of mapping throttle input from a physical device to a vJoy device. 
This setup is designed to accommodate softwares where the afterburner On/Off is assigned to key or button inputs rather than the throttle position. It maps the physical throttle's afterburner detent position to 100% on the vJoy throttle while also triggering button operations for the afterburner On/Off based on the physical throttle's position.

```lua
-- throttle.x.change: Event ID of physical throttle
-- v_throttle:        vJoyUnit object for the throttle axis of vJoy device
-- v_ab:              vJoyUnit object for the afterburner button of vJoy device
set_primary_mappings{
    {
        event = throttle.x.change,
        action = filter.duplicator{
            filter.lerp(
                v_throttle.value_setter(),
                {-50000, -50000},
                {-30000, -50000},
                {50000, 50000},
            ),
            filter.branch{
                {condition="falled", value=-44000, action=v_ab:value_setter(true)},
                {condition="exceeded", value=-39000, action=v_ab:value_setter(false)}
            }
        }
    }
}
```

## System Events
Most **Event ID**s are dynamically allocated during script processing, but some events have their Event IDs reserved by the fsmapper system even before script execution. 
These events are referred to as **System Events** and can be referenced from the [`mapper.events`](/libs/mapper/mapper_events) table.

Below is a list of currently defined system events.

|Key|Description|
|---|-----------|
|`change_aircraft`|An event that occurs when Flight Simulator starts or stops, or when the selected aircraft changes. The **Event Value** is a associative array table where the `host` key holds the running Flight Simulator software and the `aircraft` key contains the selected aircraft.<br/>Currently, fsmapper only supports Microsoft Flight Simurator 2020, so the `host` value is either `'fs2020'` or `nil`.