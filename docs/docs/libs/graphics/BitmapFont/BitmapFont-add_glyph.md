---
sidebar_position: 1
---

# BitmapFont:add_glyph()
```lua
BitmapFont:add_glyph(param_table)
BitmapFont:add_glyph(code_point, bitmap)
```
This method adds a glyph correnspoinding to a code point.


## Prameters
|Parameter|Type|Description|
|-|-|-|
|`param_table`|table|This parameter is in associative array table format, meaning it's specified by keys rather than parameter positions. See the [Parameters Table](#parameters-table) section.|
|`code_point`|string|Specifies the glyph to add and the corresponding Unicode code point using a Lua string of length 1.<br/>For example, if you want to add a glyph for U+004D, specify `'M'`.
|`bitmap`|[`Bitmap`](/libs/graphics/Bitmap)|Bitmap that represents the glyph.


## Parameters Table
|Key|Type|Description|
|-|-|-|
|`code_point`|string|Specifies the glyph to add and the corresponding Unicode code point using a Lua string of length 1.<br/>For example, if you want to add a glyph for U+004D, specify `'M'`.
|`bitmap`|[`Bitmap`](/libs/graphics/Bitmap)|Bitmap that represents the glyph.


## Return Values
This method doesn't return any value.