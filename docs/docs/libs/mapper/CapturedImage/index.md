---
sidebar_position: 1
id: CapturedImage_index
---

# CapturedImage object
CapturedImage object is a view element object used to display a partial rectangular region of the window image captured by the [`WindowImageStreamer`](/libs/mapper/WindowImageStreamer/) in a view.

:::warning Note
When a viewport includes a view containing a [`CapturedImage`](/libs/mapper/CapturedImage/) view element, 
touch and mouse interaction messages in the transparent areas of the view will no longer be passed to the underlying window. 
In other words, interaction with background windows via touch or mouse actions will be disabled. 
This can cause issues if you are simultaneously using a [`CapturedWindow`](/libs/mapper/CapturedWindow) view element for touch-based instruments, such as the Garmin G3X Touch.

Take caution when using both [`CapturedImage`](/libs/mapper/CapturedImage/) and [`CapturedWindow`](/libs/mapper/CapturedWindow) view elements within the same viewport.
:::

## Constructors
|Constructor|
|---|
|[`WindowImageStreamer:create_view_element()`](/libs/mapper/WindowImageStreamer/WindowImageStreamer-create_view_element/)

## SeeAlso
- [Virtual Instrument Panel](/guide/virtual_instrument_panel)
- [How to integrate DCS Instruments](/guide/virtual_instrument_panel/dcs)
- [`Viewport:register_view()`](/libs/mapper/Viewport/Viewport-register_view)
