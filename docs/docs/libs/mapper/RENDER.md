---
sidebar_position: 34
---

# RENDER()
```lua
function RENDER(context, value)
```
Renderer function for a [`Canvas`](/libs/mapper/Canvas) view element defined by the user.

This function is called every time rendering is required for the region associated with the [`Canvas`](/libs/mapper/Canvas) object it is registered with.
Specifically, it is invoked when the view becomes the current view, when the [`value`](/libs/mapper/Canvas/Canvas_value) property of the [`Canvas`](/libs/mapper/Canvas) object is updated, or when rendering is deemed necessary by fsmapper due to the impact of the redraw process of other [`Canvas`](/libs/mapper/Canvas) objects.

## Prameters
|Parameter|Type|Description|
|-|-|-|
|`context`|[`RenderingContext`](/libs/graphics/RenderingContext)|Rendering context that uses the View to which the [`Canvas`](/libs/mapper/Canvas) object belongs as the output destination. This rendering context has its origin and scaling factor set according to the coordinate system of the [`Canvas`](/libs/mapper/Canvas).
|`value`|Any type|The [`value`](/libs/mapper/Canvas/Canvas_value) property of the [`Canvas`](/libs/mapper/Canvas) object is passed.

## Return Values
fsmapper does not reference the return value of this function.