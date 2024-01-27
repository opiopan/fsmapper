---
sidebar_position: 1
---

# Path:add_figure()
```lua
Path:add_figure(param_table)
```
This method adds a figure to the path.

:::info Note
The definition of a figure extensively employs two-dimensional vectors.
A two-dimensional vector is represented by an array table with two elements, such as `{10, 20}`. 
This structure will be denoted as **VEC2D** in the following context.
:::


## Prameters
|Parameter|Type|Description|
|-|-|-|
|`param_table`|table|This parameter is in associative array table format, meaning it's specified by keys rather than parameter positions. See the [Parameters Table](#parameters-table) section.|


## Parameters Table
|Key|Type|Description|
|-|-|-|
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


## Return Values
This method doesn't return any value.