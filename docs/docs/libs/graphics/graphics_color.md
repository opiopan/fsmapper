---
sidebar_position: 1
---

# graphics.color()
```lua
graphics.color(color_name[, opacity)
graphics.color(r, g, b[, opacity])
```
This function creates a solid color brush object.


## Prameters
|Parameter|Type|Description|
|-|-|-|
|`color_name`|string|Web standard color specification.<br/>It can be done in the form of [color name similar to CSS](https://www.w3.org/wiki/CSS/Properties/color/keywords) such as `'Red'`, or hexadecimal expression such as `'#ff0000'`.
|`r`|numeric|Red component specified as an integer between `0` and `255`.
|`g`|numeric|Green component specified as an integer between `0` and `255`.
|`b`|numeric|Blue component specified as an integer between `0` and `255`.
|`opacity`|numeric|Opacity specified as an numeric between '0' and '1'.<br/>The default is `1`.


## Return Values
This function returns a [`Color`](/libs/graphics/Color) object.