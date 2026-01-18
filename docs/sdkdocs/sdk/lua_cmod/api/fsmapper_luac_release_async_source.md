# fsmapper_luac_release_async_source function

The **`fsmapper_luac_release_async_source`** function releases an asynchronous event source previously created by [`fsmapper_luac_create_async_source`](./fsmapper_luac_create_async_source).

Releasing the [`FSMAPPER_LUAC_ASYNC_SOURCE`](../data_types) handle informs fsmapper that the calling module no longer needs the associated resources, including any Lua objects bound to the source.

## Syntax

```c
void fsmapper_luac_release_async_source(
    FSMAPPER_LUAC_ASYNC_SOURCE source,
    lua_State* L
);
```

## Parameters

| Parameter | Type | Description |
| --- | --- | --- |
| source | [`FSMAPPER_LUAC_ASYNC_SOURCE`](../data_types) | An asynchronous event source handle to be released. This value must have been returned by [`fsmapper_luac_create_async_source`](./fsmapper_luac_create_async_source). |
| L | [`lua_State*`](https://www.lua.org/manual/5.4/manual.html#lua_State) | A pointer to the Lua state used to release the asynchronous event source. This must be the Lua environment created and managed by fsmapper. |

## Return Values

This function does not return a value.

## Remarks

- This function releases fsmapper-side resources associated with the specified [`FSMAPPER_LUAC_ASYNC_SOURCE`](../data_types), including Lua objects bound to the source.
- The [`lua_State`](https://www.lua.org/manual/5.4/manual.html#lua_State) specified by `L` must be the same Lua environment that was used when the asynchronous event source was created.
  Internally, fsmapper stores a reference to the Lua object specified by [`event_provider_arg`](./fsmapper_luac_create_async_source#parameters) in the Lua registry at creation time.
  This function uses the provided [`lua_State`](https://www.lua.org/manual/5.4/manual.html#lua_State) to remove that registry reference; therefore, passing a different or unrelated Lua state is not permitted.
- Each asynchronous event source must be released exactly once. Releasing the same handle more than once results in undefined behavior.
- An asynchronous event source must be released no later than script termination.
- Lua does not provide an explicit notification when a script terminates. A common pattern is to associate the asynchronous event source with a userdata object and call this function from the userdata’s [`__gc`](https://www.lua.org/manual/5.4/manual.html#2.5.3) metamethod.
- After this function is called, the released [`FSMAPPER_LUAC_ASYNC_SOURCE`](../data_types) handle must not be used with any other asynchronous event source API.

## See Also

- [`FSMAPPER_LUAC_ASYNC_SOURCE`](../data_types)
- [`fsmapper_luac_create_async_source`](./fsmapper_luac_create_async_source) function
- [`fsmapper_luac_async_source_signal`](./fsmapper_luac_async_source_signal) function
- [Asynchronous Event Source](../async)
