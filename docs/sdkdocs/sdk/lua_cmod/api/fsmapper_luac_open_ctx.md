# fsmapper_luac_open_ctx function

The **`fsmapper_luac_open_ctx`** function creates a service context required for using fsmapper’s service APIs from a Lua C module.

The returned [`FSMAPPER_LUAC_CTX`](../data_types) handle is used when emitting events or outputting log messages from native code, including calls made from worker threads.
This function establishes the association between the Lua C module and fsmapper for service-level operations.

## Syntax

```c
FSMAPPER_LUAC_CTX fsmapper_luac_open_ctx(
    lua_State* L,
    const char* mod_name
);
```

## Parameters

| Parameter | Type | Description |
| --- | --- | --- |
| L | [`lua_State*`](https://www.lua.org/manual/5.4/manual.html#lua_State) | A pointer to the Lua state in which the Lua C module is executing. This must be the Lua environment created and managed by fsmapper—that is, the same Lua state used to execute the fsmapper Lua script. Passing an unrelated or independently created Lua state is not supported and will cause this function to fail.
| mod_name | `const char*` | A human-readable name identifying the Lua C module. This name is used by fsmapper for diagnostic purposes, such as tagging console log messages. |

## Return Values

Returns a valid [`FSMAPPER_LUAC_CTX`](../data_types) handle on success.

If the service context cannot be created, this function returns a null pointer.

## Remarks

- The returned [`FSMAPPER_LUAC_CTX`](../data_types) represents a **service context**, not the lifetime of a Lua module itself.
- Multiple service contexts may coexist simultaneously; creating more than one context (for example, one per userdata) is fully supported.
- The `mod_name` parameter does not need to match the name used with Lua’s [`require`](https://www.lua.org/manual/5.4/manual.html#pdf-require), but it should be chosen so that log messages clearly identify the originating module.
- A service context created by this function **must** be released using [`fsmapper_luac_release_ctx`](./fsmapper_luac_release_ctx) no later than script termination.
- Lua does not provide an explicit notification when a script terminates. A common pattern is to associate the service context with a userdata object and release it from the userdata’s [`__gc`](https://www.lua.org/manual/5.4/manual.html#2.5.3) metamethod.
- The returned handle is opaque and must not be dereferenced directly by the module.
- If a Lua state not associated with fsmapper is passed, this function returns a null pointer.

## See Also

- [`FSMAPPER_LUAC_CTX`](../data_types)
- [`fsmapper_luac_release_ctx`](./fsmapper_luac_release_ctx) function
- [Basic Functions](../basic)
