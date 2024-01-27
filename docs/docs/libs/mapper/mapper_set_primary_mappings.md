---
sidebar_position: 11
---

# mapper.set_primary_mappings()
```lua
mapper.set_primary_mappings(mapping_defs)
```
This funcion sets up the primary Event-Action mappings.

:::tip
You can reset the primary Event-Action mappings by specifying an empty table, i.e. `{}`.
:::

## Prameters
|Parameter|Type|Description|
|-|-|-|
|`mapping_defs`|table|Event-Action mapping array.<br/>It specifis an array table of [Event-Action mapping definiton](#event-action-mapping-definition)

### Event-Action mapping definition
|Parameter|Type|Description|
|-|-|-|
|`event`|numeric|Specifies the event ID of the event targeted for mapping.
|`action`|function,<br/>[native-action](/guide/event-action-mapping#action)|Specifies the action targeted for mapping.<br/>A Lua function in [this format](/libs/mapper/ACTION) or [native-action](/guide/event-action-mapping#action) can be specified as a action.


## Return Values
This function doesn't return any value.