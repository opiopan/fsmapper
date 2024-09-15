---
sidebar_position: 3
---

# dcs.register_chunk()
```lua
dcs.register_chunk(chunk)
```
This function registers a Lua chunk to be executed within the DCS World process.
The Lua chunk registered with this function can be executed within the DCS World process using either [`dcs.execute_chunk()`](/libs/dcs/dcs_execute_chunk) or [`dcs.chunk_executer()`](/libs/dcs/dcs_chunk_executer).


:::warning note
While fsmapper uses [Lua 5.4](https://www.lua.org/manual/5.4/manual.html), DCS World is built with [Lua 5.1](https://www.lua.org/manual/5.1/manual.html). The string passed to this function must comply with [Lua 5.1](https://www.lua.org/manual/5.1/manual.html) specification.
:::

## Parameters
|Parameter|Type|Description|
|-|-|-|
|`chunk`|string|A string representing the Lua chunk


## Return Values
This function returns an ID that uniquely identifies the registered chunk.

## See Also
- [Interaction with DCS](/guide/dcs)
- [Executing a Lua chunk](/guide/dcs#executing-a-lua-chunk)
- [`dcs.clear_chunk()`](/libs/dcs/dcs_clear_chunk)
- [`dcs.execute_chunk()`](/libs/dcs/dcs_execute_chunk)
- [`dcs.chunk_executer()`](/libs/dcs/dcs_chunk_executer)
