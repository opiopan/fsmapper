---
sidebar_position: 3
---

# graphics.rounded_rectangle()
```lua
graphics.rounded_rectangle(param_table)
graphics.rounded_rectangle(x, y, width, height, radius_x, radius_y)
```
This function creates a [`SimpleGeometry`](/libs/graphics/SimpleGeometry) object as a rounded rectangle.


## Parameters
|Parameter|Type|Description|
|-|-|-|
|`param_table`|table|This parameter is in associative array table format, meaning it's specified by keys rather than parameter positions. See the [Parameters Table](#parameters-table) section.|
|`x`|number|X-coordinate of the top-left corner of the rectangle.
|`y`|number|Y-coordinate of the top-left corner of the rectangle.
|`width`|number|Width of the rectangle.
|`heigth`|number|Height of the rectangle.
|`radius_x`|number|X-radius for the quarter ellipse that is drawn to replace every corner of the rectangle.
|`radius_y`|number|Y-radius for the quarter ellipse that is drawn to replace every corner of the rectangle.


## Parameters Table
|Key|Type|Description|
|-|-|-|
|`x`|number|X-coordinate of the top-left corner of the rectangle.
|`y`|number|Y-coordinate of the top-left corner of the rectangle.
|`width`|number|Width of the rectangle.
|`heigth`|number|Height of the rectangle.
|`radius_x`|number|X-radius for the quarter ellipse that is drawn to replace every corner of the rectangle.
|`radius_y`|number|Y-radius for the quarter ellipse that is drawn to replace every corner of the rectangle.


## Return Values
This function returns a [`SimpleGeometry`](/libs/graphics/SimpleGeometry) object.

## See Also
- [Graphics](/guide/graphics)
- [Simple Geometry](/guide/graphics#simple-geometry)
- [`SimpleGeometry`](/libs/graphics/SimpleGeometry/)
- [`RenderingContext:draw_geometry()`](/libs/graphics/RenderingContext/RenderingContext-draw_geometry)
- [`RenderingContext:fill_geometry()`](/libs/graphics/RenderingContext/RenderingContext-fill_geometry)
