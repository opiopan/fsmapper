---
sidebar_position: 12
---

# RenderingContext:draw_bitmap()
```lua
RenderingContext:draw_bitmap(param_table)
RenderingContext:draw_bitmap(bitmap[, x, y[, width, height[, angle[, scale]]])
```
This method draws a bitmap.


## Parameters
|Parameter|Type|Description|
|-|-|-|
|`param_table`|table|This parameter is in associative array table format, meaning it's specified by keys rather than parameter positions. See the [Parameters Table](#parameters-table) section.|
|`bitmap`|[`Bitmap`](/libs/graphics/Bitmap)|The bitmap for drawing.
|`x`|number|Draw the bitmap's origin to coincide with this parameter's position in the target space.<br/>The default is `0`.
|`y`|number|Draw the bitmap's origin to coincide with this parameter's position in the target space.<br/>The default is `0`.
|`width`|number|Perform horizontal scaling to match the bitmap's width to the specified value in the target space.The default is the bitmap's width.
|`height`|number|Perform vertical scaling to match the bitmap's height to the specified value in the target space.The default is the bitmap's height.
|`angle`|number|Rotate counterclockwise by the specified angle in degrees around the bitmap's origin.The default is `0`.
|`scale`|number|The scaling factor.<br/>If `width` and `height` are specified together, the final scaling factor becomes the cumulative value.


## Parameters Table
|Key|Type|Description|
|-|-|-|
|`bitmap`|[`Bitmap`](/libs/graphics/Bitmap)|The bitmap for drawing.
|`x`|number|Draw the bitmap's origin to coincide with this parameter's position in the target space.<br/>The default is `0`.
|`y`|number|Draw the bitmap's origin to coincide with this parameter's position in the target space.<br/>The default is `0`.
|`width`|number|Perform horizontal scaling to match the bitmap's width to the specified value in the target space.The default is the bitmap's width.
|`height`|number|Perform vertical scaling to match the bitmap's height to the specified value in the target space.The default is the bitmap's height.
|`angle`|number|Rotate counterclockwise by the specified angle in degrees around the bitmap's origin.The default is `0`.
|`scale`|number|The scaling factor.<br/>If `width` and `height` are specified together, the final scaling factor becomes the cumulative value.


## Return Values
This method doesn't return any value.

## See Also
- [Bitmap](/guide/graphics#bitmap)
- [`Bitmap`](/libs/graphics/Bitmap)
