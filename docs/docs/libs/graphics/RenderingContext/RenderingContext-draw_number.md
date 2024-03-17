---
sidebar_position: 14
---

# RenderingContext:draw_number()
```lua
RenderingContext:draw_number(param_table)
RenderingContext:draw_number(value[, x[, y[, width[, height[, horizontal_alignment[, vertical_alignment]]]]]])
```
This method renders a formated string of a numeric value using the font and brush set in the rendering context.


## Parameters
|Parameter|Type|Description|
|-|-|-|
|`param_table`|table|This parameter is in associative array table format, meaning it's specified by keys rather than parameter positions. See the [Parameters Table](#parameters-table) section.|
|`value`|numeric|Numeric value to be rendered.
|`x`|numeric|The size of the bounding box in which the text is drawn. It specifies the X-coordinate of the top-left corner of the bounding box.<br/>The default value is 0.
|`y`|numeric|The size of the bounding box in which the text is drawn. It specifies the Y-coordinate of the top-left corner of the bounding box.<br/>The default value is 0.
|`width`|numeric|The width of the bounding box in which the text is drawn.<br/>The default value is positive infinity.
|`height`|numeric|The height of the bounding box in which the text is drawn.<br/>The default value is positive infinity.
|`horizontal_alignment`|string|Specifies how to horizontally align the rendering result within the bounding box specified by the `x`, `y`, `width`, and `height` parameters.<br/>It specifies either of `left`, `center`, or `right`.<br/>The default is `left`.
|`vertical_alignment`|string|Specifies how to vertically align the rendering result within the bounding box specified by the `x`, `y`, `width`, and `height` parameters.<br/>It specifies either of `top`, `center`, or `bottom`.<br/>The default is `top`.

## Parameters Table
|Key|Type|Description|
|-|-|-|
|`value`|numeric|Numeric value to be rendered.
|`x`|numeric|The size of the bounding box in which the text is drawn. It specifies the X-coordinate of the top-left corner of the bounding box.<br/>The default value is 0.
|`y`|numeric|The size of the bounding box in which the text is drawn. It specifies the Y-coordinate of the top-left corner of the bounding box.<br/>The default value is 0.
|`width`|numeric|The width of the bounding box in which the text is drawn.<br/>The default value is positive infinity.
|`height`|numeric|The height of the bounding box in which the text is drawn.<br/>The default value is positive infinity.
|`horizontal_alignment`|string|Specifies how to horizontally align the rendering result within the bounding box specified by the `x`, `y`, `width`, and `height` parameters.<br/>It specifies either of `left`, `center`, or `right`.<br/>The default is `left`.
|`vertical_alignment`|string|Specifies how to vertically align the rendering result within the bounding box specified by the `x`, `y`, `width`, and `height` parameters.<br/>It specifies either of `top`, `center`, or `bottom`.<br/>The default is `top`.
|`precision`|numeric|The overall precision, i.e., number of digits including both the integer and fractional parts.<br/>If this parameter is omitted, there is no constraint on precision.
|`fraction_precision`|numeric|Precision of the fractional part.<br/>If this parameter is omitted, there is no constraint on the precision of the fractional part.
|`leading_zero`|boolean|Specification of whether to pad the leading zeros if the number of digits is less than the specified precision.<br/>If `true` is specified for this parameter, leading zeros will be padded.<br/>The default is `false`.

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
- [`RenderingContext:draw_string()`](/libs/graphics/RenderingContext/RenderingContext-draw_string)
