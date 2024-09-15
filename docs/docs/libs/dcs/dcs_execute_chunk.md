---
sidebar_position: 5
---

# dcs.execute_chunk()
```lua
dcs.execute_chunk(chunk-id[, argument])
```
This function executes a registered Lua chunk by [`dcs.register_chunk()`](/libs/dcs/dcs_register_chunk) within the DCS World process.


## Parameters
|Parameter|Type|Description|
|-|-|-|
|`chunk-id`|number|Specify the chunk ID returned by [`dcs.register_chunk()`](/libs/dcs/dcs_register_chunk)
|`argument`|number<br/>string|Argument for chunk execution. This parameter is optional.


## Return Values
This function doesn't return any value.

## See Also
- [Interaction with DCS](/guide/dcs)
- [Executing a Lua chunk](/guide/dcs#executing-a-lua-chunk)
- [`dcs.register_chunk()`](/libs/dcs/dcs_register_chunk)
- [`dcs.chunk_executer()`](/libs/dcs/dcs_chunk_executer)
