---
sidebar_position: 4
---

# graphics.ellipse()
```lua
graphics.ellipse(param_table)
graphics.ellipse(x, y, radius_x, radius_y)
```
This function creates a [`SimpleGeometry`](/libs/graphics/SimpleGeometry) object as a ellipse.


## Prameters
|Parameter|Type|Description|
|-|-|-|
|`param_table`|table|This parameter is in associative array table format, meaning it's specified by keys rather than parameter positions. See the [Parameters Table](#parameters-table) section.|
|`x`|numeric|X-coordinate of the center point of the ellipse.
|`y`|numeric|Y-coordinate of the center point of the ellipse.
|`radius_x`|numeric|X-radius of the ellipse.
|`radius_y`|numeric|Y-radius of the ellipse.


## Parameters Table
|Key|Type|Description|
|-|-|-|
|`x`|numeric|X-coordinate of the center point of the ellipse.
|`y`|numeric|Y-coordinate of the center point of the ellipse.
|`radius_x`|numeric|X-radius of the ellipse.
|`radius_y`|numeric|Y-radius of the ellipse.


## Return Values
This function returns a [`SimpleGeometry`](/libs/graphics/SimpleGeometry) object.
