---
sidebar_position: 6
---

# dcs.chunk_executer()
```lua
dcs.chunk_executer(chunk-id[, argument])
```

This function creates a native-action to executes a registered Lua chunk by [`dcs.register_chunk()`](/libs/dcs/dcs_register_chunk) within the DCS World process.


## Parameters
|Parameter|Type|Description|
|-|-|-|
|`chunk-id`|number|Specify the chunk ID returned by [`dcs.register_chunk()`](/libs/dcs/dcs_register_chunk)
|`argument`|number<br/>string|Argument for chunk execution. If this parameter is not specified, the value of the event passed to the native action will be used.


## Return Values
This function returns a native-action.

## See Also
- [Interaction with DCS](/guide/dcs)
- [Executing a Lua chunk](/guide/dcs#executing-a-lua-chunk)
- [`dcs.register_chunk()`](/libs/dcs/dcs_register_chunk)
- [`dcs.execute_chunk()`](/libs/dcs/dcs_execute_chunk)
