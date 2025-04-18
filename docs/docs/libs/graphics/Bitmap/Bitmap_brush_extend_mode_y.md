---
sidebar_position: 5
---

# Bitmap.brush_extend_mode_y
```lua
Bitmap.brush_extend_mode_y
```

Specifies how a brush paints areas outside of the bitmap with one of the following values with the [`brush_extend_mode_x`](/libs/graphics/Bitmap/Bitmap_brush_extend_mode_x) property.

- `'clamp'`:<br/>
    Repeat the edge pixels of the bitmap for all regions outside the bitmap area.
- `'wrap'`:<br/>
    Repeat the bitmap content. This is the default.
- `'mirror'`:<br/>
    The same as `'wrap'`, except that alternate tiles of the bitmap content are flipped.

This property is used when a bitmap is employed as a brush.

## Type
string

## See Also
- [Bitmap Brush](/guide/graphics#bitmap-brush)
