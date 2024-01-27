---
sidebar_position: 18
---

# mapper.view_elements.canvas()
```lua
mapper.view_elements.canvas(param_table)
```
This function creates a [`Canvas`](/libs/mapper/Canvas) view element object.


## Prameters
|Parameter|Type|Description|
|-|-|-|
|`param_table`|table|This parameter is in associative array table format, meaning it's specified by keys rather than parameter positions. See the [Parameters Table](#parameters-table) section.|


## Parameters Table
|Key|Type|Description|
|-|-|-|
|renderer|function|Specifies the [renderer](/libs/mapper/RENDER) function for the [`Canvas`](/libs/mapper/Canvas) object to be created.<br/>This parameter is required.
|value|Any type|Specifies the initial value of the value property for the [`Canvas`](/libs/mapper/Canvas) object to be created.<br/>The default is `nil`.
|translucency|boolean|This parameter indicates whether the [`Canvas`](/libs/mapper/Canvas) object has transparent or translucent areas. When multiple [`Canvas`](/libs/mapper/Canvas) objects overlap, if this parameter is set to `false`, it avoids redrawing the [`Canvas`](/libs/mapper/Canvas) objects in the background, reducing processing costs. If set to true, it redraws overlapping [`Canvas`](/libs/mapper/Canvas) objects from the back, ensuring correct rendering of translucent results.<br/>The default is `false`.
|`logical_width`|numeric|Specifies the logical width of the canvas.<br/>It determines the aspect ratio of the canvas, along with `logical_height`, and sets the unit length in the logical coordinate system. If this parameter is specified, the coordinate system for the canvas will be absolute coordinate system.<br/> This parameter and `aspect_ratio` are mutually exclusive.
|`logical_height`|numeric|Specifies the logical height of the canvas. Refer to the description for `lgical_width`.
|`aspect_ratio`|numeric|Specifies the aspect ratio of the canvas.<br/>If this parameter is specified, the coordinate system for the canvas will be relative coordinate system.<br/>This parameter and the pair of `logical_width` and `logical_height` are mutually exclusive.
|`horizontal_alignment`|string|Specifies how to align the canvas horizontally when the aspect ratio of the view element region and the aspect ratio of the canvas differ. It specifies either of `center`, `left`, or `right`.<br/>The default is `center`.
|`vertical_alignment`|string|Specifies how to align the canvas vertically when the aspect ratio of the view element retion and the aspect ratio of the canvas differ. It specifies either of `center`, `top`, or `bottom`.<br/>The default is `center`.


## Return Values
This function returns a [`Canvas`](/libs/mapper/Canvas) object.