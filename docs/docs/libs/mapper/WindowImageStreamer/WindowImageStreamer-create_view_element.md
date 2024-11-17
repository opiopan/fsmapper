---
sidebar_position: 1
---

# WindowImageStreamer:create_view_element()
```lua
WindowImageStreamer:create_view_element([param_table])
```
This method creates a [`CapturedImage`](/libs/mapper/CapturedImage/) view element to show a portion of captured image.

:::warning Note
When a viewport includes a view containing a [`CapturedImage`](/libs/mapper/CapturedImage/) view element, 
touch and mouse interaction messages in the transparent areas of the view will no longer be passed to the underlying window. 
In other words, interaction with background windows via touch or mouse actions will be disabled. 
This can cause issues if you are simultaneously using a [`CapturedWindow`](/libs/mapper/CapturedWindow) view element for touch-based instruments, such as the Garmin G3X Touch.

Take caution when using both [`CapturedImage`](/libs/mapper/CapturedImage/) and [`CapturedWindow`](/libs/mapper/CapturedWindow) view elements within the same viewport.
:::

## Parameters
|Parameter|Type|Description|
|-|-|-|
|`param_table`|table|This parameter is in associative array table format, meaning it's specified by keys rather than parameter positions. See the [Parameters Table](#parameters-table) section.<br/>If this parameter is omitted, it is equivalent to specifying an empty table `{}`.|


## Parameters Table
|Key|Type|Description|
|-|-|-|
|`x`|number|Specifies the x-coordinate of the left edge of the rectangular area within the captured image by the [`WindowImageStreamer`](/libs/mapper/WindowImageStreamer/) object to be displayed as a [`CapturedImage`](/libs/mapper/CapturedImage/) view element, in pixel units.<br/>If this parameter is omitted, it defaults to `0`.
|`y`|number|Specifies the y-coordinate of the top edge of the rectangular area within the captured image by the [`WindowImageStreamer`](/libs/mapper/WindowImageStreamer/) object to be displayed as a [`CapturedImage`](/libs/mapper/CapturedImage/) view element, in pixel units.<br/>If this parameter is omitted, it defaults to `0`.
|`width`|number|Specifies the width of the rectangular area within the captured image by the [`WindowImageStreamer`](/libs/mapper/WindowImageStreamer/) object to be displayed as a [`CapturedImage`](/libs/mapper/CapturedImage/) view element, in pixel units.<br/>If this parameter is set to `0` or omitted, it defaults to the width of the capture target window.
|`height`|number|Specifies the height of the rectangular area within the captured image by the [`WindowImageStreamer`](/libs/mapper/WindowImageStreamer/) object to be displayed as a [`CapturedImage`](/libs/mapper/CapturedImage/) view element, in pixel units.<br/>If this parameter is set to `0` or omitted, it defaults to the height of the capture target window.
|`horizontal_alignment`|string|Specifies how to align the captured image horizontally when the aspect ratio of the view element region and the aspect ratio specified by `width` and `height`. It specifies either of `center`, `left`, or `right`.<br/>The default is `center`.
|`vertical_alignment`|string|Specifies how to align the captured image vertically when the aspect ratio of the view element retion and the aspect ratio specified by `width` and `height` differ. It specifies either of `center`, `top`, or `bottom`.<br/>The default is `center`.

## Return Values
This function returns a [`CapturedImage`](/libs/mapper/CapturedImage/) object.

## See Also
- [Virtual Instrument Panel](/guide/virtual_instrument_panel)
- [How to integrate DCS Instruments](/guide/virtual_instrument_panel/dcs)
- [`Viewport:register_view()`](/libs/mapper/Viewport/Viewport-register_view)
- [`CapturedImage`](/libs/mapper/CapturedImage/)