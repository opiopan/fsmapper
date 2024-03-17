---
sidebar_position: 5
---

# Viewport:set_mappings()
```lua
Viewport:set_mappings(mapping_defs)
```
This function sets Event-Action mappings associated with the viewport.

:::tip
You can reset the Event-Action mappings associated with the viewport by specifying an empty table, i.e. `{}`.
:::

## Parameters
|Parameter|Type|Description|
|-|-|-|
|`mapping_defs`|table|Event-Action mapping array.<br/>It specifis an array table of [Event-Action mapping definiton](/libs/mapper/mapper_set_primary_mappings#event-action-mapping-definition)

## Return Values
This function doesn't return any value.

## See Also
- [Event-Action Mapping](/guide/event-action-mapping)
- [`Viewport:add_mappings()`](/libs/mapper/Viewport/Viewport-add_mappings)
