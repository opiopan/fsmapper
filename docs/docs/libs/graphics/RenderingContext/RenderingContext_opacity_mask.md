---
sidebar_position: 4
---

# RenderingContext.opacity_mask
```lua
RenderingContext.opacity_mask
```

This property stores a [bitmap](/libs/graphics/Bitmap) used as the opacity mask.<br/>
The opacity mask is the mask image used when drawing shapes.
The final alpha value of the rendered result is a product of the alpha value specified in this property and the bitmap provided.

## Type
[`Bitmap`](/libs/graphics/Bitmap)