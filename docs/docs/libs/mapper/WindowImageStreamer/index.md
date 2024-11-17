---
sidebar_position: 1
id: WindowImageStreamer_index
---

# WindowImageStreamer object
WindowImageStreamer object continuously captures and retains the display content of another process’s window in real time using the [Windows.Graphics.Capture](https://learn.microsoft.com/en-us/uwp/api/windows.graphics.capture?view=winrt-26100) API.
By including the [`CapturedImage`](/libs/mapper/CapturedImage/) view element, generated through the [`create_view_element()`](/libs/mapper/WindowImageStreamer/WindowImageStreamer-create_view_element/) method of this object, as a component of the view, you can display a selected rectangular portion of the captured area within the view.

This mechanism was originally introduced to dynamically control the position, size, and visibility of DCS World’s exported MFCDs and similar instruments as flexible components within fsmapper’s views, comparable to the level of flexibility provided for Microsoft Flight Simulator. However, it can also be used to display content from applications other than DCS World within the view.

## Constructors
|Constructor|
|---|
|[`mapper.window_image_streamer()`](/libs/mapper/mapper_window_image_streamer/)

## Methods
|Name|Description|
|-|-|
|[`WindowImageStreamer:create_view_element()`](/libs/mapper/WindowImageStreamer/WindowImageStreamer-create_view_element/)|Create a view element to show a portion of captured image|

## See Also
- [Virtual Instrument Panel](/guide/virtual_instrument_panel)
- [How to integrate DCS Instruments](/guide/virtual_instrument_panel/dcs)
