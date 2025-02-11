---
sidebar_position: 25
---

# mapper.enumerate_display_info()
```lua
mapper.enumerate_display_info()
```
This function enumerates information for all displays.

## Return Values

This function returns an array of associative array tables described below. 
The number of elements in the array corresponds to the number of displays connected to the system, with the display number serving as the array index.

|Key|Type|Description|
|---|----|-----------|
|id|number|Display number
|x|number|The x-coordinate value of the top-left corner of the display in screen space
|y|number|The y-coordinate value of the top-left corner of the display in screen space
|width|number|The width of the display
|height|number|The height of the display
|name|string|Display name
|adapter|string|Name of the graphics adapter to which the display is connected

:::warning NOTE
Displays set to “**Disconnect this display**” in display settings are not included in the table returned by this function.
In other words, the elements corresponding to display numbers marked as “**Disconnect this display**” will be `nil`.

Note that in such cases, using [`ipairs()`](https://www.lua.org/manual/5.4/manual.html#pdf-ipairs) on the returned table may not enumerate all elements.
To iterate over all elements, use [`pairs()`](https://www.lua.org/manual/5.4/manual.html#pdf-pairs) instead.
Similarly, the [`#` operator](https://www.lua.org/manual/5.4/manual.html#3.4.7) cannot be used to obtain the correct number of elements (i.e., the number of active displays).
:::

## See Also
- [Virtual Instrument Panel](/guide/virtual_instrument_panel)
