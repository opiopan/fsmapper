---
sidebar_position: 13
---

# RenderingContext:draw_string()
```lua
RenderingContext:draw_string(param_table)
RenderingContext:draw_string(string[, x, y])
```
This method renders text using the font set in the rendering context.


## Prameters
|Parameter|Type|Description|
|-|-|-|
|`param_table`|table|This parameter is in associative array table format, meaning it's specified by keys rather than parameter positions. See the [Parameters Table](#parameters-table) section.|
|`string`|string|String to be rendered.
|`x`|numeric|The placement location of the rendering result.<br/>It specifies the X-coordinate of the top-left corner of the bounding box of the rendering result.
|`y`|numeric|The placement location of the rendering result.<br/>It specifies the Y-coordinate of the top-left corner of the bounding box of the rendering result.


## Parameters Table
|Key|Type|Description|
|-|-|-|
|`string`|string|String to be rendered.
|`x`|numeric|The placement location of the rendering result.<br/>It specifies the X-coordinate of the top-left corner of the bounding box of the rendering result.
|`y`|numeric|The placement location of the rendering result.<br/>It specifies the Y-coordinate of the top-left corner of the bounding box of the rendering result.


## Return Values
This method doesn't return any value.