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
|x|number|The x-coordinate value of the top-left corner of the display in screen space
|y|number|The y-coordinate value of the top-left corner of the display in screen space
|width|number|The width of the display
|height|number|The height of the display

## See Also
- [Virtual Instrument Panel](/guide/virtual_instrument_panel)
