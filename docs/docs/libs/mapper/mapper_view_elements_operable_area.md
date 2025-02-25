---
sidebar_position: 17
---

# mapper.view_elements.operable_area()
```lua
mapper.view_elements.operable_area(param_table)
```
This function creates a [`OperableArea`](/libs/mapper/OperableArea) view element object.


## Parameters
|Parameter|Type|Description|
|-|-|-|
|`param_table`|table|This parameter is in associative array table format, meaning it's specified by keys rather than parameter positions. See the [Parameters Table](#parameters-table) section.|


## Parameters Table
|Key|Type|Description|
|-|-|-|
|reaction_color|[`Color`](/libs/graphics/Color) |The region of this view element is filled with the specified color by this parameter while being touched.<br/>The default is the color created by `mapper.color('Cyan', 0.3)`
|round_ratio|number|The ratio to the short side of the corner radius of the rectangle. This parameter is specified when the shape of the filled area, while being touched, is a rounded rectangle instead of a rectangle.<br/>When specifying 0.5 and the width and height of the view element are equal, the filled area becomes a circle.<br/>The default is `0`.
|event_tap|number|Specifies the Event ID to be triggered upon detecting a tap action.
|event_flick_up|number|Specifies the Event ID to be triggered upon detecting a upward flick action.
|event_flick_down|number|Specifies the Event ID to be triggered upon detecting downward flick action.
|event_flick_right|number|Specifies the Event ID to be triggered upon detecting rightward flick action.
|event_flick_left|number|Specifies the Event ID to be triggered upon detecting leftward flick action.
|event_rotate_clockwise|number|**CURRENTRY NOT IMPLEMENTED**<br/>Specifies the Event ID to be triggered upon detecting clockwise rotation.
|event_rotate_counterclockwise|number|**CURRENTRY NOT IMPLEMENTED**<br/>Specifies the Event ID to be triggered upon detecting counterclockwise rotation.


## Return Values
This function returns a [`OperableArea`](/libs/mapper/OperableArea) object.

## See Also
- [Virtual Instrument Panel](/guide/virtual_instrument_panel)
- [View Element](/guide/virtual_instrument_panel#view-element)
- [Handle Touch Event](/guide/virtual_instrument_panel#handle-touch-event)
- [`Viewport:register_view()`](/libs/mapper/Viewport/Viewport-register_view)
- [`OperableArea`](/libs/mapper/OperableArea)
