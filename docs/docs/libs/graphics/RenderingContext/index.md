---
sidebar_position: 1
id: RenderingContext_index
---

# RenderingContext object
RenderingContext object is created for each rendering target, it enables graphic rendering to the specified destination through method calls within the rendering context. Additionally, the rendering context retains context-specific elements such as line width and fill colors that affect the rendering process.

## Constructors
|Constructor|
|---|
|[`graphics.rendering_context()`](/libs/graphics/graphics_rendering_context)

## Properties
|Name|Description|
|-|-|
|[```RenderingContext.brush```](/libs/graphics/RenderingContext/RenderingContext_brush)|Current brush|
|[```RenderingContext.font```](/libs/graphics/RenderingContext/RenderingContext_font)|Current font|
|[```RenderingContext.stroke_width```](/libs/graphics/RenderingContext/RenderingContext_stroke_width)|Current stroke width|
|[```RenderingContext.opacity_mask```](/libs/graphics/RenderingContext/RenderingContext_opacity_mask)|Bitmap used as opacity mask|

## Methods
|Name|Description|
|-|-|
|[```RenderingContext:finish_rendering()```](/libs/graphics/RenderingContext/RenderingContext-finish_rendering)|Finalize the drawing process|
|[```RenderingContext:set_brush()```](/libs/graphics/RenderingContext/RenderingContext-set_brush)|Set a brush to the context|
|[```RenderingContext:set_font()```](/libs/graphics/RenderingContext/RenderingContext-set_font)|Set a font to the context|
|[```RenderingContext:set_stroke_width()```](/libs/graphics/RenderingContext/RenderingContext-set_stroke_width)|Set a stroke width to the context|
|[```RenderingContext:set_opacity_mask()```](/libs/graphics/RenderingContext/RenderingContext-set_opacity_mask)|Set a bitmap used as opacity mask to the context|
|[```RenderingContext:draw_geometry()```](/libs/graphics/RenderingContext/RenderingContext-draw_geometry)|Draw a geometry|
|[```RenderingContext:fill_geometry()```](/libs/graphics/RenderingContext/RenderingContext-fill_geometry)|Fill a geometry|
|[```RenderingContext:draw_bitmap()```](/libs/graphics/RenderingContext/RenderingContext-draw_bitmap)|Draw a bitmap|
|[```RenderingContext:draw_string()```](/libs/graphics/RenderingContext/RenderingContext-draw_string)|Draw a string|
|[```RenderingContext:draw_number()```](/libs/graphics/RenderingContext/RenderingContext-draw_number)|Draw a formated string of a numeric value|
|[```RenderingContext:fill_rectangle()```](/libs/graphics/RenderingContext/RenderingContext-fill_rectangle)|Fill a rectangle|
