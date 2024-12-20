---
sidebar_position: 4
---

# graphics.ellipse()
```lua
graphics.ellipse(param_table)
graphics.ellipse(x, y, radius_x, radius_y)
```
This function creates a [`SimpleGeometry`](/libs/graphics/SimpleGeometry) object as a ellipse.


## Parameters
|Parameter|Type|Description|
|-|-|-|
|`param_table`|table|This parameter is in associative array table format, meaning it's specified by keys rather than parameter positions. See the [Parameters Table](#parameters-table) section.|
|`x`|number|X-coordinate of the center point of the ellipse.
|`y`|number|Y-coordinate of the center point of the ellipse.
|`radius_x`|number|X-radius of the ellipse.
|`radius_y`|number|Y-radius of the ellipse.


## Parameters Table
|Key|Type|Description|
|-|-|-|
|`x`|number|X-coordinate of the center point of the ellipse.
|`y`|number|Y-coordinate of the center point of the ellipse.
|`radius_x`|number|X-radius of the ellipse.
|`radius_y`|number|Y-radius of the ellipse.


## Return Values
This function returns a [`SimpleGeometry`](/libs/graphics/SimpleGeometry) object.

## See Also
- [Graphics](/guide/graphics)
- [Simple Geometry](/guide/graphics#simple-geometry)
- [`SimpleGeometry`](/libs/graphics/SimpleGeometry/)
- [`RenderingContext:draw_geometry()`](/libs/graphics/RenderingContext/RenderingContext-draw_geometry)
- [`RenderingContext:fill_geometry()`](/libs/graphics/RenderingContext/RenderingContext-fill_geometry)
