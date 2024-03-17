---
sidebar_position: 7
---

# graphics.system_font()
```lua
graphics.system_font(param_table)
```
This function creates a system font object representing the fonts registered in the Windows operating system.


## Parameters
|Parameter|Type|Description|
|-|-|-|
|`param_table`|table|This parameter is in associative array table format, meaning it's specified by keys rather than parameter positions. See the [Parameters Table](#parameters-table) section.|


## Parameters Table
|Key|Type|Description|
|-|-|-|
|`family_name`|string|The name of the font family. This parameter is required.
|`weight`|numeric|A value that indicates the font weight. A numeric value from 1 to 999 can be specified for this parameter.<br/>The default value is 400.
|`style`|string|Specifies the font style as either `normal`, `oblique`, or `italic`.<br/>The default is `normal`.
|`height`|numeric|The logical size of the font in the logical unit of the [rendering context](/guide/graphics#rendering-context).<br/>When the [`SystemFont`](/libs/graphics/SystemFont) object returned by this function is set on a rendering context associated with a bitmap, it is interpreted in pixel units. Conversely, when set on a rendering context associated with a view, it is interpreted according to [the coordinate system of the Canvas view element](/guide/virtual_instrument_panel#coordinate-system). <br/>This parametaer is required.


## Return Values
This function returns a [`SystemFont`](/libs/graphics/SystemFont) object.

## See Also
- [Graphics](/guide/graphics)
- [Font](/guide/graphics#font)
- [`SystemFont`](/libs/graphics/SystemFont)
- [`RenderingContext.font`](/libs/graphics/RenderingContext/RenderingContext_font)
- [`RenderingContext:set_font()`](/libs/graphics/RenderingContext/RenderingContext-set_font)
- [`RenderingContext:draw_string())`](/libs/graphics/RenderingContext/RenderingContext-draw_string)
- [`RenderingContext:draw_number())`](/libs/graphics/RenderingContext/RenderingContext-draw_number)
