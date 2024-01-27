---
sidebar_position: 6
---

# Bitmap.brush_interpolation_mode
```lua
Bitmap.brush_interpolation_mode
```
Specifies the algorithm that is used when images are scaled or rotated with ether of following values.

- `'nearest_neighbor'`:<br/>
    Use the exact color of the nearest bitmap pixel to the current rendering pixel.
- `'linear'`:<br/>
    Interpolate a color from the four bitmap pixels that are the nearest to the rendering pixel. This is the default.

This property is used when a bitmap is employed as a brush.

## Type
string
