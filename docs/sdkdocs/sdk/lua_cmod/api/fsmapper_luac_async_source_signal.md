# fsmapper_luac_async_source_signal function

The **`fsmapper_luac_async_source_signal`** function signals fsmapper that an asynchronous event source has pending event(s) to be emitted.

This function is **thread-safe** and may be called from any thread, including worker threads created by a Lua C module. When fsmapper processes the signal, it invokes the event provider associated with the source on the Lua script execution thread to obtain the event ID(s) and value(s).

## Syntax

```c
void fsmapper_luac_async_source_signal(
    FSMAPPER_LUAC_ASYNC_SOURCE source
);
```

## Parameters

| Parameter | Type | Description |
| --- | --- | --- |
| source | [`FSMAPPER_LUAC_ASYNC_SOURCE`](../data_types) | An asynchronous event source handle to be signaled. |

## Return Values

This function does not return a value.

## Remarks

- This function may be called from **any thread** and does not require access to the Lua stack.
- The associated event provider function is invoked by fsmapper on the Lua script execution thread.
- The event(s) emitted are determined solely by the event provider’s return values, as described in [Asynchronous Event Source](../async).
- After an asynchronous event source is released using [`fsmapper_luac_release_async_source`](./fsmapper_luac_release_async_source), the handle must not be used with this function.

## See Also

- [`FSMAPPER_LUAC_ASYNC_SOURCE`](../data_types)
- [`fsmapper_luac_create_async_source`](./fsmapper_luac_create_async_source) function
- [`fsmapper_luac_release_async_source`](./fsmapper_luac_release_async_source) function
- [Asynchronous Event Source](../async)
