---
sidebar_position: 7
---

# mapper.register_event()
```lua
mapper.register_event(name)
```
This function registers an event.

## Prameters
|Parameter|Type|Description|
|-|-|-|
|`name`|string|Specifies the name to assign to the event.<br/>The name is used for display in the message console. It doesn't need to be unique among registered events.|


## Return Values
This function returns a numeric value representing the event ID.

## See Also
- [Event Action Mapping](/guide/event-action-mapping)
- [`mapper.unregister_event()`](/libs/mapper/mapper_unregister_event)
- [`mapper.get_event_name()`](/libs/mapper/mapper_get_event_name)