---
sidebar_position: 1
---

# graphics.color()
```lua
graphics.color(color_name[, opacity)
graphics.color(r, g, b[, opacity])
```
This function creates a solid color brush object.


## Parameters
|Parameter|Type|Description|
|-|-|-|
|`color_name`|string|Web standard color specification.<br/>It can be done in the form of [color name similar to CSS](https://www.w3.org/wiki/CSS/Properties/color/keywords) such as `'Red'`, or hexadecimal expression such as `'#ff0000'`.
|`r`|number|Red component specified as an integer between `0` and `255`.
|`g`|number|Green component specified as an integer between `0` and `255`.
|`b`|number|Blue component specified as an integer between `0` and `255`.
|`opacity`|number|Opacity specified as an number between '0' and '1'.<br/>The default is `1`.


## Return Values
This function returns a [`Color`](/libs/graphics/Color) object.

## See Also
- [Graphics](/guide/graphics)
- [Solid Color](/guide/graphics#solid-color)
- [`Color`](/libs/graphics/Color)
- [`RenderingContext.brush`](/libs/graphics/RenderingContext/RenderingContext_brush)
- [`RenderingContext:set_brush()`](/libs/graphics/RenderingContext/RenderingContext-set_brush)
