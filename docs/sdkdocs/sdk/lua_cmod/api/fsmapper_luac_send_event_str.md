# fsmapper_luac_send_event_str function

The **`fsmapper_luac_send_event_str`** function emits an fsmapper event with a string value from a Lua C module.

This function is part of the Basic Functions and is **thread-safe**, allowing events to be emitted from any thread, including worker threads created by the module.

## Syntax

```c
void fsmapper_luac_send_event_str(
    FSMAPPER_LUAC_CTX ctx,
    FSMAPPER_EVENT_ID evid,
    const char* data
);
```

## Parameters

| Parameter | Type | Description |
| --- | --- | --- |
| ctx | [`FSMAPPER_LUAC_CTX`](../data_types) | A service context identifying the Lua C module that emits the event. |
| evid | [`FSMAPPER_EVENT_ID`](../data_types) | The event ID to be emitted. Event IDs are typically assigned using [`mapper.register_event()`](/libs/mapper/mapper_register_event) in Lua scripts. |
| data | `const char*` | A null-terminated UTF-8 string to be used as the event value. |

## Return Values

This function does not return a value.

## Remarks

- This function may be called from **any thread** and does not require access to the Lua stack.
- The string value passed via `data` is copied internally by fsmapper; the caller may safely release or reuse the memory after the function returns.
- The emitted event is processed by fsmapper’s event-action mapping mechanism. The event ID must correspond to an event registered in the Lua environment, either by the script or by the module calling [`mapper.register_event()`](/libs/mapper/mapper_register_event) from within Lua execution.

## See Also

- [`FSMAPPER_LUAC_CTX`](../data_types)
- [`FSMAPPER_EVENT_ID`](../data_types)
- [`fsmapper_luac_putLog`](./fsmapper_luac_putLog) function
- [`fsmapper_luac_send_event`](./fsmapper_luac_send_event) function
- [`fsmapper_luac_send_event_int`](./fsmapper_luac_send_event_int) function
- [`fsmapper_luac_send_event_float`](./fsmapper_luac_send_event_float) function
- [Basic Functions](../basic)
