---
sidebar_position: 2
---

# graphics.rectangle()
```lua
graphics.rectangle(param_table)
graphics.rectangle(x, y, width, heigth)
```
This function creates a [`SimpleGeometry`](/libs/graphics/SimpleGeometry) object as a rectangle.

## Parameters
|Parameter|Type|Description|
|-|-|-|
|`param_table`|table|This parameter is in associative array table format, meaning it's specified by keys rather than parameter positions. See the [Parameters Table](#parameters-table) section.|
|`x`|number|X-coordinate of the top-left corner of the rectangle.
|`y`|number|Y-coordinate of the top-left corner of the rectangle.
|`width`|number|Width of the rectangle.
|`heigth`|number|Height of the rectangle.


## Parameters Table
|Key|Type|Description|
|-|-|-|
|`x`|number|X-coordinate of the top-left corner of the rectangle.
|`y`|number|Y-coordinate of the top-left corner of the rectangle.
|`width`|number|Width of the rectangle.
|`heigth`|number|Height of the rectangle.


## Return Values
This function returns a [`SimpleGeometry`](/libs/graphics/SimpleGeometry) object.

## See Also
- [Graphics](/guide/graphics)
- [Simple Geometry](/guide/graphics#simple-geometry)
- [`SimpleGeometry`](/libs/graphics/SimpleGeometry/)
- [`RenderingContext:draw_geometry()`](/libs/graphics/RenderingContext/RenderingContext-draw_geometry)
- [`RenderingContext:fill_geometry()`](/libs/graphics/RenderingContext/RenderingContext-fill_geometry)
