---
sidebar_position: 3
---

# graphics.rounded_rectangle()
```lua
graphics.rounded_rectangle(param_table)
graphics.rounded_rectangle(x, y, width, height, radius_x, radius_y)
```
This function creates a [`SimpleGeometry`](/libs/graphics/SimpleGeometry) object as a rounded rectangle.


## Prameters
|Parameter|Type|Description|
|-|-|-|
|`param_table`|table|This parameter is in associative array table format, meaning it's specified by keys rather than parameter positions. See the [Parameters Table](#parameters-table) section.|
|`x`|numeric|X-coordinate of the top-left corner of the rectangle.
|`y`|numeric|Y-coordinate of the top-left corner of the rectangle.
|`width`|numeric|Width of the rectangle.
|`heigth`|numeric|Height of the rectangle.
|`radius_x`|numeric|X-radius for the quarter ellipse that is drawn to replace every corner of the rectangle.
|`radius_y`|numeric|Y-radius for the quarter ellipse that is drawn to replace every corner of the rectangle.


## Parameters Table
|Key|Type|Description|
|-|-|-|
|`x`|numeric|X-coordinate of the top-left corner of the rectangle.
|`y`|numeric|Y-coordinate of the top-left corner of the rectangle.
|`width`|numeric|Width of the rectangle.
|`heigth`|numeric|Height of the rectangle.
|`radius_x`|numeric|X-radius for the quarter ellipse that is drawn to replace every corner of the rectangle.
|`radius_y`|numeric|Y-radius for the quarter ellipse that is drawn to replace every corner of the rectangle.


## Return Values
This function returns a [`SimpleGeometry`](/libs/graphics/SimpleGeometry) object.
