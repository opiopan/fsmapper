---
sidebar_position: 2
---

# Path:fix()
```lua
Path:fix()
```
This method finalizes the path to make it drawable.<br/>
Once this function is called, adding additional figures becomes disabled.
Also, when drawing a path with [`RenderingContext:draw_geometry()`](/libs/graphics/RenderingContext/RenderingContext-draw_geometry) or filling a path with [`RenderingContext:fill_geometry()`](/libs/graphics/RenderingContext/RenderingContext-fill_geometry), this method is automatically called.


## Return Values
This method doesn't return any value.