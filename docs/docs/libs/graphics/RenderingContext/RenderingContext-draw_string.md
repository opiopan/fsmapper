---
sidebar_position: 13
---

# RenderingContext:draw_string()
```lua
RenderingContext:draw_string(param_table)
RenderingContext:draw_string(string[, x[, y[, width[, height[, horizontal_alignment[, vertical_alignment]]]]]])
```
This method renders text using the font and brush set in the rendering context.


## Parameters
|Parameter|Type|Description|
|-|-|-|
|`param_table`|table|This parameter is in associative array table format, meaning it's specified by keys rather than parameter positions. See the [Parameters Table](#parameters-table) section.|
|`string`|string|String to be rendered.
|`x`|numeric|The size of the bounding box in which the text is drawn. It specifies the X-coordinate of the top-left corner of the bounding box.<br/>The default value is 0.
|`y`|numeric|The size of the bounding box in which the text is drawn. It specifies the Y-coordinate of the top-left corner of the bounding box.<br/>The default value is 0.
|`width`|numeric|The width of the bounding box in which the text is drawn.<br/>The default value is positive infinity.
|`height`|numeric|The height of the bounding box in which the text is drawn.<br/>The default value is positive infinity.
|`horizontal_alignment`|string|Specifies how to horizontally align the rendering result within the bounding box specified by the `x`, `y`, `width`, and `height` parameters.<br/>It specifies either of `left`, `center`, or `right`.<br/>The default is `left`.
|`vertical_alignment`|string|Specifies how to vertically align the rendering result within the bounding box specified by the `x`, `y`, `width`, and `height` parameters.<br/>It specifies either of `top`, `center`, or `bottom`.<br/>The default is `top`.


## Parameters Table
|Key|Type|Description|
|-|-|-|
|`string`|string|String to be rendered.
|`x`|numeric|The size of the bounding box in which the text is drawn. It specifies the X-coordinate of the top-left corner of the bounding box.<br/>The default value is 0.
|`y`|numeric|The size of the bounding box in which the text is drawn. It specifies the Y-coordinate of the top-left corner of the bounding box.<br/>The default value is 0.
|`width`|numeric|The width of the bounding box in which the text is drawn.<br/>The default value is positive infinity.
|`height`|numeric|The height of the bounding box in which the text is drawn.<br/>The default value is positive infinity.
|`horizontal_alignment`|string|Specifies how to horizontally align the rendering result within the bounding box specified by the `x`, `y`, `width`, and `height` parameters.<br/>It specifies either of `left`, `center`, or `right`.<br/>The default is `left`.
|`vertical_alignment`|string|Specifies how to vertically align the rendering result within the bounding box specified by the `x`, `y`, `width`, and `height` parameters.<br/>It specifies either of `top`, `center`, or `bottom`.<br/>The default is `top`.

:::warning Note
If the font set in the rendering context is a [`BitmapFont`](/libs/graphics/BitmapFont) object, text wrapping and alignment changes within the bounding box are not possible. 
The following parameters are ignored for [`BitmapFont`](/libs/graphics/BitmapFont) objects.
- `width`
- `height`
- `horizontal_alignment`
- `vertical_alignment`
:::

## Return Values
This method doesn't return any value.

## See Also
- [Rendering Context](/guide/graphics#rendering-context)
- [Font](/guide/graphics#font)
- [Brush](/guide/graphics#brush)
- [`SystemFont`](/libs/graphics/SystemFont)
- [`BitmapFont`](/libs/graphics/BitmapFont)
- [`RenderingContext.font`](/libs/graphics/RenderingContext/RenderingContext_font)
- [`RenderingContext.brush`](/libs/graphics/RenderingContext/RenderingContext_brush)
- [`RenderingContext:draw_number()`](/libs/graphics/RenderingContext/RenderingContext-draw_number)
