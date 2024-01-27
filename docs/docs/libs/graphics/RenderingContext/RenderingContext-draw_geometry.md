---
sidebar_position: 10
---

# RenderingContext:draw_geometry()
```lua
RenderingContext:draw_geometry(param_table)
RenderingContext:draw_geometry(geometry, x, y[, angle[, scale]])
```
This function draws the outline of the specified geometry using following properties of the rendering context.
- [`brush`](/libs/graphics/RenderingContext/RenderingContext_brush)
- [`stroke_width`](/libs/graphics/RenderingContext/RenderingContext_stroke_width)
- [`opacity_mask`](/libs/graphics/RenderingContext/RenderingContext_opacity_mask)

## Prameters
|Parameter|Type|Description|
|-|-|-|
|`param_table`|table|This parameter is in associative array table format, meaning it's specified by keys rather than parameter positions. See the [Parameters Table](#parameters-table) section.|
|`geometry`|[`Geometry`](/guide/graphics#geometry)|The geometry object for drawing. It specifies [`SimpleGeometry`](/libs/graphics/SimpleGeometry) of [`Path`](/libs/graphics/Path).
|`x`|numeric|Draw the gemometry's origin to coincide with this parameter's position in the target space.<br/>The default is `0`.
|`y`|numeric|Draw the geometry's origin to coincide with this parameter's position in the target space.<br/>The default is `0`.
|`angle`|numeric|Rotate counterclockwise by the specified angle in degrees around the geometry's origin.The default is `0`.
|`scale`|numeric|The scaling factor. The default is `1`.


## Parameters Table
|Key|Type|Description|
|-|-|-|
|`geometry`|[`Geometry`](/guide/graphics#geometry)|The geometry object for drawing. It specifies [`SimpleGeometry`](/libs/graphics/SimpleGeometry) of [`Path`](/libs/graphics/Path).
|`x`|numeric|Draw the gemometry's origin to coincide with this parameter's position in the target space.<br/>The default is `0`.
|`y`|numeric|Draw the geometry's origin to coincide with this parameter's position in the target space.<br/>The default is `0`.
|`angle`|numeric|Rotate counterclockwise by the specified angle in degrees around the geometry's origin.The default is `0`.
|`scale`|numeric|The scaling factor. The default is `1`.


## Return Values
This method doesn't return any value.