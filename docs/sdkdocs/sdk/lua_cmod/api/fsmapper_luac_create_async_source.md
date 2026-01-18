# fsmapper_luac_create_async_source function

The **`fsmapper_luac_create_async_source`** function creates an asynchronous event source that enables a Lua C module to emit events whose values are arbitrary Lua objects.

An asynchronous event source can be signaled from any thread using [`fsmapper_luac_async_source_signal`](./fsmapper_luac_async_source_signal). When fsmapper processes the signal, it invokes the specified event provider function on the Lua script execution thread to obtain one or more events to emit.

## Syntax

```c
FSMAPPER_LUAC_ASYNC_SOURCE fsmapper_luac_create_async_source(
    FSMAPPER_LUAC_CTX ctx,
    lua_State* L,
    lua_CFunction event_provider,
    int event_provider_arg
);
```

## Parameters

| Parameter | Type | Description |
| --- | --- | --- |
| ctx | [`FSMAPPER_LUAC_CTX`](../data_types) | A service context identifying the Lua C module that creates the asynchronous event source. |
| L | [`lua_State*`](https://www.lua.org/manual/5.4/manual.html#lua_State) | A pointer to the Lua state used to create the asynchronous event source. This must be the Lua environment created and managed by fsmapper—that is, the same Lua state used to execute the fsmapper Lua script. |
| event_provider | [`lua_CFunction`](https://www.lua.org/manual/5.4/manual.html#lua_CFunction) | A Lua C function that is invoked on the Lua script execution thread to provide event ID(s) and value(s) when the source is signaled.<br/>For more details, see [Event Provider Semantics](../async#event-provider-semantics)
| event_provider_arg | `int` | The stack index of a Lua object on the Lua stack referenced by `L`. The module must push the Lua value onto the stack before calling this function and specify its stack position here. The referenced Lua object is passed as an argument to the event provider function and remains associated with the asynchronous event source until it is released. Specify `0` if no argument should be passed to the event provider. |

## Return Values

Returns a valid [`FSMAPPER_LUAC_ASYNC_SOURCE`](../data_types) handle on success.

If the asynchronous event source cannot be created, this function returns a null pointer.

## Remarks

- This function associates the specified `event_provider` and the Lua object referenced by `event_provider_arg` with the created asynchronous event source.
- The returned [`FSMAPPER_LUAC_ASYNC_SOURCE`](../data_types) handle is opaque and must not be dereferenced directly by the module.
- The returned handle must be released using [`fsmapper_luac_release_async_source`](./fsmapper_luac_release_async_source) no later than script termination.
- Lua does not provide an explicit notification when a script terminates. A common pattern is to associate the service context with a userdata object and release it from the userdata’s [`__gc`](https://www.lua.org/manual/5.4/manual.html#2.5.3) metamethod.
- The event provider function must follow Lua C API conventions and return event information using one of the supported return formats described in [Asynchronous Event Source](../async).

## See Also

- [`FSMAPPER_LUAC_ASYNC_SOURCE`](../data_types)
- [`FSMAPPER_LUAC_CTX`](../data_types)
- [`fsmapper_luac_async_source_signal`](./fsmapper_luac_async_source_signal) function
- [`fsmapper_luac_release_async_source`](./fsmapper_luac_release_async_source) function
- [Asynchronous Event Source](../async)
