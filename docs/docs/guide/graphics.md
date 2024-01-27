---
sidebar_position: 6
---

# Graphics
This section covers the graphic rendering capabilities of fsmapper.
The graphic rendering capabilities of fsmapper are provided as the [graphics library](/libs/graphics).

## Rendering Context
The rendering context ([`RenderingContext`](/libs/graphics/RenderingContext)) in fsmapper forms the central concept and functionality within its graphic rendering capabilities.
Created for each rendering target, it enables graphic rendering to the specified destination through [method](/libs/graphics/RenderingContext#methods) calls within the rendering context. Additionally, the rendering context retains context-specific elements such as line width and fill colors that affect the rendering process.

The targets for rendering within fsmapper include views and bitmaps. <br/>
As explained in the [**Render on the View**](/guide/virtual_instrument_panel#render-on-the-view), the rendering context for outputting to a view is created by fsmapper and passed as the first argument to the [renderer](/libs/mapper/RENDER) of the [`Canvas`](/libs/mapper/Canvas) view element.
Users cannot generate this within Lua scripts.<br/>
To create a rendering context targeting a bitmap, use [`graphics.rendering_context()`](/libs/graphics/graphics_rendering_context).

```lua
local bitmap = grphics.bitmap(100, 100)
local context = graphics.rendering_context(bitmap)
```

The device context holds the following properties that influence the behavior of each drawing method.

|Property|Type|Description|
|--------|----|-----------|
|`brush`|[`Blush`](#brush)|A brush refers to the color or pattern used for filling shapes or outlining their contours.
|`stroke_width`|numeric|This is used as the width of lines when drawing the outlines of shapes.
|`opacity_mask`|[`Bitmap`](#bitmap)|This is the mask image used when drawing shapes. The final alpha value of the rendered result is a product of the alpha value specified in the `opacity_mask` and the bitmap provided.
|`font`|[`Font`](#font)|This is the font object used for text rendering.

The rendering context provides the following drawing methods.

|Category|Methods|
|--------|-------|
|Drawing Geometry|[`draw_geometry()`](/libs/graphics/RenderingContext/RenderingContext-draw_geometry)<br/>[`fill_geometry()`](/libs/graphics/RenderingContext/RenderingContext-fill_geometry)
|Drawing Bitmap|[`draw_bitmap()`](/libs/graphics/RenderingContext/RenderingContext-draw_bitmap)
|Drawing Text|[`draw_string())`](/libs/graphics/RenderingContext/RenderingContext-draw_string)<br/>[`draw_number()`](/libs/graphics/RenderingContext/RenderingContext-draw_number)
|Others|[`fill_rectangle()`](/libs/graphics/RenderingContext/RenderingContext-fill_rectangle)

The drawing operations executed through the rendering context are finalized by calling the [`finish_rendering()`](/libs/graphics/RenderingContext/RenderingContext-finish_rendering) method. 

```lua
context:finish_rendering()
```

:::info Note
The rendering context passed to the [renderer](/libs/mapper/RENDER) of the [`Canvas`](/libs/mapper/Canvas) is finalized by fsmapper, so there's no need to call this method from Lua scripts. If called, it will be ignored.
:::

## Bitmap
Bitmaps are the most commonly used building blocks when rendering virtual instrument panels.
The [`Bitmap`](/libs/graphics/Bitmap) object is provided to be used for these bitmap-related operations.

The simplest way to create a bitmap object is by loading a bitmap image from a file.

```lua
local button1 = graphics.bitmap('asset/button1.png')
```

Rather than statically preparing a file, you can dynamically create a bitmap parametrically.
Invoking [`graphics.bitmap()`](/libs/graphics/graphics_bitmap) with specified dimensions generates a bitmap object initialized with transparency. 
Creating a rendering context with this object as the output destination allows dynamic generation of bitmap images through rendering operations.

```lua
-- Generate empty bitmap
local circle = graphics.bitmap(100, 100)

-- Fill a circle with blue on the bitmap
local rctx = graphics.rendering_context(circle)
rctx.brush = graphics.color('blue')
local circle = graphics.ellipse{x=50, y=50, radius_x=50, radius_y=50}
rctx:draw_fill_geometry(circle)
rctx:finish_rendering()
```

The final way to generate a bitmap object involves creating a portion of the bitmap as a separate bitmap object using the [`create_partial_bitmap()`](/libs/graphics/Bitmap/Bitmap-create_partial_bitmap) method of the [`Bitmap`](/libs/graphics/Bitmap) object. Bitmaps created using this method share the buffer that holds the bitmap data with the original bitmap object, reducing concerns about memory consumption.<br/>
This method is useful for preparing multiple image components for various canvases as a single bitmap and generating partial bitmaps for each canvas, just as in [this file](https://github.com/opiopan/fsmapper/blob/main/samples/practical/assets/a320_buttons.png) or [this file](https://github.com/opiopan/fsmapper/blob/main/samples/practical/assets/instrument_parts.png).

```lua
local entire_bitmap = graphics.bitmap('assets/parts_library.png')
local partial_bitmap = entire_bitmap:create_partial_bitmap(0, 0, 50, 50)
```

### Supported File Format
fsmapper can load the following types of bitmap image files.

- BMP
- GIF
- ICO
- JPEG
- PNG
- TIFF
- HD Photo

### Entire Opacity
The [`Bitmap`](/libs/graphics/Bitmap) object contains an alpha value indicating the opacity of each pixel. However, you can specify the overall opacity of the entire bitmap using the [`opacity`](/libs/graphics/Bitmap/Bitmap_opacity) property.
The opacity of each pixel is determined by the product of the pixel's alpha value and the [`opacity`](/libs/graphics/Bitmap/Bitmap_opacity) property.

### Drawing Bitmap
The method used to draw a Bitmap is [`RenderingContext:draw_bitmap()`](/libs/graphics/RenderingContext/RenderingContext-draw_bitmap).

This method includes parameters for translation and rotation, both of which act relative to the origin of the bitmap.
At the bitmap's creation, the origin is at the top-left corner.
To set a different point within the bitmap space as the new origin, you can use [`Bitmap:set_origin()`](/libs/graphics/Bitmap/Bitmap-set_origin).

```lua
-- Set the origin to the center of bitmap
local dial = graphics.bitmap(asset/dial.jpg)
dial:set_origin(dial.width / 2, dial.height / 2)
```

The parameters that can be specified for [`RenderingContext:draw_bitmap()`](/libs/graphics/RenderingContext/RenderingContext-draw_bitmap) are as follows.

|Key|Type|Description|
|---|----|-----------|
|`bitmap`|[`Bitmap`](/libs/graphics/Bitmap)|**REQUIRED PARAMETER**<br/>The bitmap for drawing.
|`x`|numeric|Draw the bitmap's origin to coincide with this parameter's position in the target space.<br/>The default is `0`.
|`y`|numeric|Draw the bitmap's origin to coincide with this parameter's position in the target space.<br/>The default is `0`.
|`width`|numeric|Perform horizontal scaling to match the bitmap's width to the specified value in the target space.The default is the bitmap's width.
|`height`|numeric|Perform vertical scaling to match the bitmap's height to the specified value in the target space.The default is the bitmap's height.
|`angle`|numeric|Rotate counterclockwise by the specified angle in degrees around the bitmap's origin.The default is `0`.
|`scale`|numeric|The scaling factor.<br/>If `width` and `height` are specified together, the final scaling factor becomes the cumulative value.

Here's an example of a [`renderer`](/libs/mapper/RENDER) that displays a bitmap rotated by the canvas's `value` degrees and positioned at (100, 100) on the canvas at half its size.

```lua
local bitmap = graphics.bitmap('assets/knob.png')
local renderer = function (rctx, value)
    rctx:draw_bitmap(bitmap=bitmap, x=100, y=100, scale=0.5, angle=value)
end
```

## Brush
The objects that can be set as a brushe in the rendering context are [`Color`](/libs/graphics/Color) objects representing solid colors or [`Bitmap`](/libs/graphics/Bitmap) objects.

### Solid Color
A [`Color`](/libs/graphics/Color) object representing a solid color is created using [`graphics.color()`](/libs/graphics/graphics_color). 
Color specification can be done in the form of a [color name similar to CSS](https://www.w3.org/wiki/CSS/Properties/color/keywords), hexadecimal values, or by specifying RGB values numerically.

Each of the following three examples generates a Color object representing the same color.

```lua
local color1 = graphics.color('DarkCyan')
local color2 = graphics.color('#008b8b')
local color3 = graphics.color(0,139,139)
```

In any of these cases, you can also create a translucent color by adding an argument that represents opacity.

```lua
local color1 = graphics.color('DarkCyan', 0.5)
local color2 = graphics.color('#008b8b', 0.5)
local color3 = graphics.color(0,139,139, 0.5)
```

### Bitmap Brush
Bitmap objects can be used as brushes.
When using a bitmap as a brush, the following properties of [`Bitmap`](/libs/graphics/Bitmap) affect the fill effect.

|Property|Description|
|--------|-----------|
|`brush_extend_mode_x`<br/>`brush_extend_mode_y`|Specifies how a brush paints areas outside of the bitmap with one of the following values.<br/><br/>`'clamp'`: Repeat the edge pixels of the bitmap for all regions outside the bitmap area.<br/><br/>`'wrap'`: Repeat the bitmap content. This is the default.<br/><br/>`'mirror'`: The same as `'wrap'`, except that alternate tiles of the bitmap content are flipped.
|`brush_interpolation_mode`|Specifies the algorithm that is used when images are scaled or rotated with ether of following values.<br/><br/>`'nearest_neighbor'`: Use the exact color of the nearest bitmap pixel to the current rendering pixel.<br/><br/>`'linear'`: Interpolate a color from the four bitmap pixels that are the nearest to the rendering pixel. This is the default.

## Geometry
Geometry is an object that defines the shape of figures.
There are [`SimpleGeometry`](/libs/graphics/SimpleGeometry) objects representing system-defined basic shapes and [`Path`](/libs/graphics/Path) objects, which allow defining complex shapes freely by combining segments such as lines and arcs.

### Simple Geometry
The [`SimpleGeometry`](/libs/graphics/SimpleGeometry) object is created using one of the following functions.

|Name|Description|
|-|-|
|[```graphics.rectangle()```](/libs/graphics/graphics_rectangle)|Create a SimpleGeometry object as a rectangle|
|[```graphics.rounded_rectangle()```](/libs/graphics/graphics_rounded_rectangle)|Create a SimpleGeometry object as a rounded rectangle|
|[```graphics.ellipse()```](/libs/graphics/graphics_ellipse)|Create a SimpleGeometry object as a ellipse|

### Path
The [`Path`](/libs/graphics/Path) object can describe complex shapes by combining segments such as lines, arcs, and Bezier curves.
The Path object is created with [`graphics.path()`](/libs/graphics/graphics_path).

```lua
local path = graphics.path()
```

The [`Path`](/libs/graphics/Path) object can be composed of multiple **figures**, and you can add one figure to the [`Path`](/libs/graphics/Path) object using [`Path:add_figure()`](/libs/graphics/Path/Path-add_figure).
A figure refers to a shape that can be represented by a single stroke.

The [`Path:add_figure()`](/libs/graphics/Path/Path-add_figure) specifies the following parameters.
:::info Note
The definition of a figure extensively employs two-dimensional vectors.
A two-dimensional vector is represented by an array table with two elements, such as `{10, 20}`. 
This structure will be denoted as **VEC2D** in the following context.
:::

|Key|Type|Description|
|---|----|-----------|
|`fill_mode`|string|Specifies the fill mode with one of the following values.<br/><br/>`'none'`: Indicating that it's a figures representing the outline without filling.<br/><br/>`'winding'`: See [this site](https://learn.microsoft.com/en-us/windows/win32/api/d2d1/ne-d2d1-d2d1_fill_mode#remarks) that explains the meaning of this mode in an easy-to-understand manner.<br/><br/>`'alternate'`: See [this site](https://learn.microsoft.com/en-us/windows/win32/api/d2d1/ne-d2d1-d2d1_fill_mode#remarks) that explains the meaning of this mode in an easy-to-understand manner.
|`from`|VEC2D|The starting point of the figure.
|`segments`|table|An array table defining segments. Each element of the array refers to the [**Segment Definition**](#segment-definition).

#### Segment Definition
The definition of segments is done using one of the following tables.

- **Line Segment**<br/>
    |Key|Type|Description|
    |---|----|-----------|
    |`to`|VEC2D|End point of the line. The starting point of the line is the endpoint of the previous segment. If it's the first segment, the `from` of the figure becomes the starting point.

- **Arc Segment**<br/>
    |Key|Type|Description|
    |---|----|-----------|
    |`to`|VEC2D|End point of the arc. The starting point of the arc is the endpoint of the previous segment. If it's the first segment, the `from` of the figure becomes the starting point.
    |`radius`|numeric|The radius of the arc.
    |`direction`|string|A value that specifies whether the arc sweep is clockwise or counterclockwise, such as `'clockwise'` or `'counterclockwise'`.
    |`arc_type`|string|A value that specifies whether the given arc is larger than 180 degrees, such as `'large'` or `'small'`

- **Bezier Curve Segment**<br/>
    |Key|Type|Description|
    |---|----|-----------|
    |`to`|VEC2D|End point of the curve. The starting point of the curve is the endpoint of the previous segment. If it's the first segment, the `from` of the figure becomes the starting point.
    |`control1`|VEC2D|A VEC2D that represents the first control point for the curve.
    |`control2`|VEC2D|A VEC2D that represents the second control point for the curve.

:::tip
[`graphics.path()`](/libs/graphics/graphics_path) accepts arguments in the same format as [`Path:add_figure()`](/libs/graphics/Path/Path-add_figure). This means that the first figure can be simultaneously registered when generating the [`Path`](/libs/graphics/Path) object."
:::

### Drawing Geometry
The drawing of geometry, much like a bitmap, allows specification of translation, rotation, and scaling. Similarly, the origin of the geometry itself can be modified using [`SimpleGeometry:set_origin()`](/libs/graphics/SimpleGeometry/SimpleGeometry-set_origin) or [`Path:set_origin()`](/libs/graphics/Path/Path-set_origin).

Where it differs from a bitmap is that geometry has two types of operations, filling ([`RenderingContext:fill_geometry()`](/libs/graphics/RenderingContext/RenderingContext-fill_geometry)) and drawing the outline ([`RenderingContext:draw_geometry()`](/libs/graphics/RenderingContext/RenderingContext-draw_geometry)). Additionally, during rendering, geometry is affected by properties of the rendering context.

The rendering context properties that affect it are as follows:

- [`brush`](/libs/graphics/RenderingContext/RenderingContext_brush)
- [`stroke_width`](/libs/graphics/RenderingContext/RenderingContext_stroke_width)
- [`opacity_mask`](/libs/graphics/RenderingContext/RenderingContext_opacity_mask)

The parameters that can be specified for [`RenderingContext:fill_geometry()`](/libs/graphics/RenderingContext/RenderingContext-fill_geometry) and [`RenderingContext:draw_geometry()`](/libs/graphics/RenderingContext/RenderingContext-draw_geometry) are as follows.

|Key|Type|Description|
|---|----|-----------|
|`geometry`|[`Geometry`](#geometry)|**REQUIRED PARAMETER**<br/>The geometry object for drawing. It specifies [`SimpleGeometry`](/libs/graphics/SimpleGeometry) of [`Path`](/libs/graphics/Path).
|`x`|numeric|Draw the gemometry's origin to coincide with this parameter's position in the target space.<br/>The default is `0`.
|`y`|numeric|Draw the geometry's origin to coincide with this parameter's position in the target space.<br/>The default is `0`.
|`angle`|numeric|Rotate counterclockwise by the specified angle in degrees around the geometry's origin.The default is `0`.
|`scale`|numeric|The scaling factor. The default is `1`.

The following is the code example for the canvas that draws the heading indicator needle with the geometry version under the [**Render on the View**](/guide/virtual_instrument_panel#render-on-the-view) heading.

```lua
-- Define a path to depict a needle
local needle = graphics.path{
    fill_mode = 'winding',
    from = {0, 50},
    segments = {
        {to = {5, 0}},
        {to = {-5, 0}, radius = 5, direction = 'clockwise', arc_type = 'small'},
        {to = {0, 50}},
    }
}

-- Define a canvas to render the needle
local needle_canvas = mapper.canvas{
    local_width = 100, local_height = 100,
    value = 0,
    renderer = function (rctx, value)
        rctx.brush = graphics.color('DarkCyan')
        rctx:fill_geometry{geometry=needle, x=50, y=50, rotation=value}
        rctx.brush = graphics.color('Black')
        rctx.stroke_width = 1.5
        rctx:draw_geometry{geometry=needle, x=50, y=50, rotation=value}
    end
}
```

## Font
The **Font** refers to the object targeted by the text-drawing methods of the rendering context.
Currently, fsmapper implements only the [`BitmapFont`](/libs/graphics/BitmapFont) object, allowing users to provide glyphs as bitmaps on a code point basis.

:::warning Note
From the start, within fsmapper's internal workings, the handling of fonts was abstracted to accommodate the Windows font system similarly.
However, integration with the Windows font system is not yet complete.
I'm planning to roll up my sleeves and get started on this soon. (as of Jan. 2024).
:::

### Bitmap Font
Generate a BitmapFont object using [`graphics.bitmap_font()`](/libs/graphics/graphics_bitmap_font) and register bitmaps representing glyphs for each code point using [`BitmapFont:add_glyph()`](/libs/graphics/BitmapFont/BitmapFont-add_glyph).
The width and height of each glyph can vary per code point based on the bitmap size. 
Additionally, adjusting the origin of the registered bitmaps allows tweaking the so-called baseline or accommodating glyphs that overlap with the previous character when rendering text.

In [this Lua module](https://github.com/opiopan/fsmapper/blob/v0.9.1/samples/practical/lib/segdisp.lua) used in [this sample script](/samples/c172), we parametrically generate a font for a 7-segment display. Please refer to it as an example of BitmapFont usage.

### Drawing Text with Font
To render text, start by setting the font within the rendering context.
Afterwards, utilize [`RenderingContext:draw_string()`](/libs/graphics/RenderingContext/RenderingContext-draw_string) to output strings, or [`RenderingContext:draw_number()`](/libs/graphics/RenderingContext/RenderingContext-draw_number) to define formatting parameters, such as precision for decimal values, when rendering numeric data.

```lua
-- Create a fixed-width font from the pre-rendered bitmap
local font_width = 24
local bitmap = graphics.bitmap('assets/fixed-width-font')
local codes='0123456789.-+*/_!abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ'
local font = graphics.bitmap_font()
for ix = 1, string.len(codes) do
    local glyph = bitmap:create_partial_bitmap((ix - 1) * font_width, 0, font_width, bitmap.height)
    font:add_glyph(string.sub(codes, ix, ix), glyph)
end

-- Define the Canvas renderer that draws the static text
local renderer = function (rctx, value)
    rctx.font = font

    -- Draw a string
    rctx:draw_string('Hello!', 0, 0)

    -- Draw a numeric value, this formats as the string '003.14'
    rctx:draw_number{
        value = 3.1415926,
        x = 0, y = 30,
        precision = 5,
        fraction_precision = 2,
        leading_zero = true
    }
end
```