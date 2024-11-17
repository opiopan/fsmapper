---
sidebar_position: 11
---

# RenderingContext:fill_geometry()
```lua
RenderingContext:fill_geometry(param_table)
RenderingContext:fill_geometry(geometry, x, y[, angle[, scale]])
```
This method fills the specified geometry using following properties of the rendering context.
- [`brush`](/libs/graphics/RenderingContext/RenderingContext_brush)
- [`stroke_width`](/libs/graphics/RenderingContext/RenderingContext_stroke_width)
- [`opacity_mask`](/libs/graphics/RenderingContext/RenderingContext_opacity_mask)

## Parameters
|Parameter|Type|Description|
|-|-|-|
|`param_table`|table|This parameter is in associative array table format, meaning it's specified by keys rather than parameter positions. See the [Parameters Table](#parameters-table) section.|
|`geometry`|[`Geometry`](/guide/graphics#geometry)|The geometry object for drawing. It specifies [`SimpleGeometry`](/libs/graphics/SimpleGeometry) of [`Path`](/libs/graphics/Path).
|`x`|number|Draw the gemometry's origin to coincide with this parameter's position in the target space.<br/>The default is `0`.
|`y`|number|Draw the geometry's origin to coincide with this parameter's position in the target space.<br/>The default is `0`.
|`angle`|number|Rotate counterclockwise by the specified angle in degrees around the geometry's origin.The default is `0`.
|`scale`|number|The scaling factor. The default is `1`.


## Parameters Table
|Key|Type|Description|
|-|-|-|
|`geometry`|[`Geometry`](/guide/graphics#geometry)|The geometry object for drawing. It specifies [`SimpleGeometry`](/libs/graphics/SimpleGeometry) of [`Path`](/libs/graphics/Path).
|`x`|number|Draw the gemometry's origin to coincide with this parameter's position in the target space.<br/>The default is `0`.
|`y`|number|Draw the geometry's origin to coincide with this parameter's position in the target space.<br/>The default is `0`.
|`angle`|number|Rotate counterclockwise by the specified angle in degrees around the geometry's origin.The default is `0`.
|`scale`|number|The scaling factor. The default is `1`.


## Return Values
This method doesn't return any value.

## See Also
- [Rendering Context](/guide/graphics#rendering-context)
- [Geometry](/guide/graphics#geometry)
- [`SimpleGeometry`](/libs/graphics/SimpleGeometry)
- [`Path`](/libs/graphics/Path)
- [`RenderingContext.brush`](/libs/graphics/RenderingContext/RenderingContext_brush)
- [`RenderingContext.stroke_width`](/libs/graphics/RenderingContext/RenderingContext_stroke_width)
- [`RenderingContext.opacity_mask`](/libs/graphics/RenderingContext/RenderingContext_opacity_mask)
- [`RenderingContext:draw_geometry()`](/libs/graphics/RenderingContext/RenderingContext-draw_geometry)
