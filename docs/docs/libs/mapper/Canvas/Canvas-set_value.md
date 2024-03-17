---
sidebar_position: 2
---

# Canvas:set_value()
```lua
Canvas:set_value(value)
```
This method sets a value for the canvas.<br/>
After calling this method, the [renderer](/libs/mapper/RENDER) registered with the canvas are invoked with the specified value as argmument.

## Parameters
|Parameter|Type|Description|
|-|-|-|
|`value`|Any type|Value to associate with the canvas


## Return Values
This method doesn't return any value.

## See Also
- [Virtual Instrument Panel](/guide/virtual_instrument_panel)
- [Render on the View](/guide/virtual_instrument_panel#render-on-the-view)
- [`Canvas.value`](/libs/mapper/Canvas/Canvas_value)
- [`Canvas:value_setter()`](/libs/mapper/Canvas/Canvas-value_setter)
- [Renderer function](/libs/mapper/RENDER)