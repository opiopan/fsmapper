---
sidebar_position: 14
---

# RenderingContext:draw_number()
```lua
RenderingContext:draw_number(param_table)
RenderingContext:draw_number(value[, x, y])
```
This method renders a formated string of a numeric value using the font set in the rendering context.


## Prameters
|Parameter|Type|Description|
|-|-|-|
|`param_table`|table|This parameter is in associative array table format, meaning it's specified by keys rather than parameter positions. See the [Parameters Table](#parameters-table) section.|
|`value`|numeric|Numeric value to be rendered.
|`x`|numeric|The placement location of the rendering result.<br/>It specifies the X-coordinate of the top-left corner of the bounding box of the rendering result.
|`y`|numeric|The placement location of the rendering result.<br/>It specifies the Y-coordinate of the top-left corner of the bounding box of the rendering result.

## Parameters Table
|Key|Type|Description|
|-|-|-|
|`value`|numeric|Numeric value to be rendered.
|`x`|numeric|The placement location of the rendering result.<br/>It specifies the X-coordinate of the top-left corner of the bounding box of the rendering result.
|`y`|numeric|The placement location of the rendering result.<br/>It specifies the Y-coordinate of the top-left corner of the bounding box of the rendering result.
|`precision`|numeric|The overall precision, i.e., number of digits including both the integer and fractional parts.<br/>If this parameter is omitted, there is no constraint on precision.
|`fraction_precision`|numeric|Precision of the fractional part.<br/>If this parameter is omitted, there is no constraint on the precision of the fractional part.
|`leading_zero`|boolean|Specification of whether to pad the leading zeros if the number of digits is less than the specified precision.<br/>If `true` is specified for this parameter, leading zeros will be padded.<br/>The default is `false`.


## Return Values
This method doesn't return any value.