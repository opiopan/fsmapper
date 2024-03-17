---
sidebar_position: 6
---

# graphics.bitmap()
```lua
graphics.bitmap(path)
graphics.bitmap(width, height)
```
This function creates a [`Bitmap`](/libs/graphics/Bitmap) object.<br/>
There are two methods to generate a [`Bitmap`](/libs/graphics/Bitmap) object, one by loading the bitmap file specified in the `path` parameter, and the other by generating a transparent bitmap of the size specified in `width` and `height`.

## Parameters
|Parameter|Type|Description|
|-|-|-|
|`path`|string|Path of the bitmap file.
|`width`|numeric|Width of the bitmap
|`height`|numeric|Width of the bitmap


## Return Values
This function returns a [`Bitmap`](/libs/graphics/Bitmap) object.

## See Also
- [Graphics](/guide/graphics)
- [Bitmap Brush](/guide/graphics#bitmap-brush)
- [`Bitmap`](/libs/graphics/Bitmap)
- [`RenderingContext.brush`](/libs/graphics/RenderingContext/RenderingContext_brush)
- [`RenderingContext:set_brush()`](/libs/graphics/RenderingContext/RenderingContext-set_brush)
- [`RenderingContext.opacity_mask`](/libs/graphics/RenderingContext/RenderingContext_opacity_mask)
- [`RenderingContext:set_opacity_mask()`](/libs/graphics/RenderingContext/RenderingContext-set_opacity_mask)