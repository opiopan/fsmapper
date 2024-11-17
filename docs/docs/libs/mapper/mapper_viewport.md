---
sidebar_position: 16
---

# mapper.viewport()
```lua
mapper.viewport(param_table)
```
This funciton registers a new [**Viewport**](/guide/virtual_instrument_panel#components-for-virtual-instrument-panel).<br/>
The viewports registered with this function are inactive until [`mapper.start_viewports()`](/libs/mapper/mapper_start_viewports) is called. Inactive viewports are not only hidden from the screen but also do not activate associated [**Event-Action mappings**](/guide/event-action-mapping) with the viewport in a paused state.


## Parameters
|Parameter|Type|Description|
|-|-|-|
|`param_table`|table|This parameter is in associative array table format, meaning it's specified by keys rather than parameter positions. See the [Parameters Table](#parameters-table) section.|


## Parameters Table
|Key|Type|Description|
|-|-|-|
|`name`|string|User-assigned name for the [`Viewport`](/libs/mapper/Viewport) object.<br/>This parameter is required.
|`displayno`|number|Specifies the display number for placing the viewport.<br/>If this parameter is omitted, the position and size of the viewport are specified in absolute coordinates relative to the entire screen space, and `x`, `y`, `width`, and `height` must be specified.
|`coordinate`|string|Cordinate system type.<br/>If `relative` is specified, the display space is treated in a relative coordinate system. This means that the values specified for `x`, `y`, `width`, and `height` are interpreted as relative to the width and height of the display.<br/>If `absolute` is specified, the display space is treated in an absolute coordinate system. This means that the values specified for `x`, `y`, `width`, and `height` are interpreted as pixel values.<br/>The default is `relative`, but it becomes `absolute` if `displayno` is omitted.
|`x`|number|X-coordinate of the top-left corner of the viewport.<br/>The default is `0`.
|`y`|number|Y-coordinate of the top-left corner of the viewport.<br/>The default is `0`.
|`width`|number|Width of the viewport.<br/>The default is the width of the display.
|`height`|number|Height of the viewport.<br/>The default is the width of the display.
|`logical_width`|number|Specifies the logical width of the viewport's active area.<br/>It determines the aspect ratio of the viewort's active area, along with `logical_height`, and sets the unit length in the logical coordinate system. If this parameter is specified, the default coordinate system for the viewport and the coordinate system inherited by the view will be absolute coordinate system.<br/> This parameter and `aspect_ratio` are mutually exclusive.
|`logical_height`|number|Specifies the logical height of the viewport's active area. Refer to the description for `lgical_width`.
|`aspect_ratio`|number|Specifies the aspect ratio of the viewport's active area.<br/>If this parameter is specified, the default coordinate system for the viewport and the coordinate system inherited by the view will be relative coordinate system.<br/>This parameter and the pair of `logical_width` and `logical_height` are mutually exclusive.
|`horizontal_alignment`|string|Specifies how to align the viewport's active area horizontally when the aspect ratio of the viewport and the aspect ratio of the active area differ. It specifies either of `center`, `left`, or `right`.<br/>The default is `center`.
|`vertical_alignment`|string|Specifies how to align the viewport's active area vertically when the aspect ratio of the viewport and the aspect ratio of the active area differ. It specifies either of `center`, `top`, or `bottom`.<br/>The default is `center`.
|`bgcolor`|[`Color`](/libs/graphics/Color), string|Background color of the viewport. In case the aspect ratio of the viewport differs from the aspect ratio of the viewport's active region, this parameter specifies the color to fill both gaps with.<br/>Both a [`Color`](/libs/graphics/Color) object and a string-formatted color name, which can be specified when creating a [`Color`](/libs/graphics/Color) object, can be used for this parameter.<br/>The default color is black.
|`ignore_transparent_touches`|boolean|If this parameter is set to `true`, touch and mouse operation messages in transparent areas of the view will not be passed to windows behind it. In other words, it prevents interaction with background windows via touch or mouse. For instance, if you are using a touch-based instrument like the Garmin G3X Touch as a [`CapturedWindow`](/libs/mapper/CapturedWindow) view element within the view, you should not set this parameter to `true`.<br/>Setting this parameter to `true` will prevent interaction with windows behind the viewport but can improve viewport display efficiency on the monitor. By default, the viewport is shown as a [Layered Window](https://learn.microsoft.com/en-us/windows/win32/winmsg/window-features#layered-windows), however, with this parameter set to true, it utilizes [DirectComposition](https://learn.microsoft.com/en-us/windows/win32/directcomp/directcomposition-portal) for rendering. Even if `GPU rendering` is specified for `Rendering Method` on the Settings page to construct the view image in GPU memory, using a Layered Window requires GPU-to-CPU data transfers per window update due to hit-testing in the transparent area. DirectComposition avoids this memory copying, thus enhancing performance.<br/>The default is `false`.

## Return Values
This function returns [`Viewport`](/libs/mapper/Viewport) object.

## See Also
- [Virtual Instrument Panel](/guide/virtual_instrument_panel)
- [`Viewport`](/libs/mapper/Viewport)
- [`mapper.start_viewports()`](/libs/mapper/mapper_start_viewports)
- [`mapper.stop_viewports()`](/libs/mapper/mapper_stop_viewports)
- [`mapper.reset_viewports()`](/libs/mapper/mapper_reset_viewports)
