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

## Prameters
|Parameter|Type|Description|
|-|-|-|
|`path`|string|Path of the bitmap file.
|`width`|numeric|Width of the bitmap
|`height`|numeric|Width of the bitmap


## Return Values
This function returns a [`Bitmap`](/libs/graphics/Bitmap) object.
