---
sidebar_position: 3
---

# Viewport:register_view()
```lua
Viewport:register_view(param_table)
```
This method registers a new view to the viewport.


## Prameters
|Parameter|Type|Description|
|-|-|-|
|`param_table`|table|This parameter is in associative array table format, meaning it's specified by keys rather than parameter positions. See the [Parameters Table](#parameters-table) section.|


## Parameters Table
|Key|Type|Description|
|-|-|-|
|name|string|User-assigned name for the view.<br/>This parameter is required.
|elements|table|Geometry information for all view elements to be placed on the view.<br/>It specifies an array table of [View Element Region Definition table](#view-element-region-definition).<br/>This parameter is required.
|mappings|table|Event-Action mapping array.<br/>It specifis an array table of [Event-Action mapping definiton](/libs/mapper/mapper_set_primary_mappings#event-action-mapping-definition).The Event-Action mappings specified by this parameter are effective only while this view is displayed as the current view.
|background|[`Color`](/libs/graphics/Color),<br/>[`Bitmap`](/libs/graphics/Bitmap)|[Brush](/guide/graphics#brush) object to fill the background of the view.<br/>If a bitmap object is specified, it will be scaled to fit the entire view both horizontally and vertically. Please note that if the aspect ratios of the view and the bitmap are different, the image may be distorted.
|`logical_width`|numeric|Specifies the logical width of the view.<br/>It determines the aspect ratio of the view, along with `logical_height`, and sets the unit length in the logical coordinate system. If this parameter is specified, the coordinate system for the view will be absolute coordinate system.<br/> This parameter and `aspect_ratio` are mutually exclusive.
|`logical_height`|numeric|Specifies the logical height of the view. Refer to the description for `lgical_width`.
|`aspect_ratio`|numeric|Specifies the aspect ratio of the view.<br/>If this parameter is specified, the coordinate system for the view will be relative coordinate system.<br/>This parameter and the pair of `logical_width` and `logical_height` are mutually exclusive.
|`horizontal_alignment`|string|Specifies how to align the view horizontally when the aspect ratio of the viewport's active area and the aspect ratio of the view differ. It specifies either of `center`, `left`, or `right`.<br/>The default is `center`.
|`vertical_alignment`|string|Specifies how to align the view vertically when the aspect ratio of the viewport's active area and the aspect ratio of the view differ. It specifies either of `center`, `top`, or `bottom`.<br/>The default is `center`.

### View Element Region Definition
|Key|Type|Description|
|-|-|-|
|object|[OperableArea](/libs/mapper/OperableArea),<br/>[Canvas](/libs/mapper/Canvas),<br/>[CapturedWindow](/libs/mapper/CapturedWindow)|Viewelement object.<br/>This parameter is required.
|`x`|numeric|X-coordinate of the top-left corner of the view element.<br/>The default is `0`.
|`y`|numeric|Y-coordinate of the top-left corner of the view element.<br/>The default is `0`.
|`width`|numeric|Width of the view element.<br/>The default is the width of the viewport's active area.
|`height`|numeric|Height of the view element.<br/>The default is the width of the viewport's active area.

## Return Values
This function returns the view ID associated with the registered view.
