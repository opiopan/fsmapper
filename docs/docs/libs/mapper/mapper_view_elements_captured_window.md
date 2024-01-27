---
sidebar_position: 19
---

# mapper.view_elements.captured_window()
```lua
mapper.view_elements.captured_window(param_table)
```
This function creates a [`CapturedWindow`](/libs/mapper/CapturedWindow) view element object.


## Prameters
|Parameter|Type|Description|
|-|-|-|
|`param_table`|table|This parameter is in associative array table format, meaning it's specified by keys rather than parameter positions. See the [Parameters Table](#parameters-table) section.|


## Parameters Table
|Key|Type|Description|
|-|-|-|
|name|string|The name given by the user to the [`CapturedWindow`](/libs/mapper/CapturedWindow) object.<br/>The name specified in this parameter will be displayed on the dashboard.<br/>This parameter is required.
|window_title|string|Title text of the window to be captured.<br/>If this parameter is omitted, the automatic window capture feature will not operate.
|omit_system_region|boolean|If this parameter is set to `true`, the title bar and window frame of the captured window will be hidden.<br/>The default is `true`.
|avoid_touch_problems|boolean|If this parameter is set to `true`, it resolves the issue of touch operations not working in FS2020's pop-out instrument windows.<br/>The default is `true`.


## Return Values
This function returens a [`CapturedWindow`](/libs/mapper/CapturedWindow) object.