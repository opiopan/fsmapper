# fsmapper_luac_send_event function

The **`fsmapper_luac_send_event`** function emits an fsmapper event without an associated value from a Lua C module.

This function is part of the Basic Functions and is **thread-safe**, allowing events to be emitted from any thread, including worker threads created by the module.

## Syntax

```c
void fsmapper_luac_send_event(
    FSMAPPER_LUAC_CTX ctx,
    FSMAPPER_EVENT_ID evid
);
```

## Parameters

| Parameter | Type | Description |
| --- | --- | --- |
| ctx | [`FSMAPPER_LUAC_CTX`](../data_types) | A service context identifying the Lua C module that emits the event. |
| evid | [`FSMAPPER_EVENT_ID`](../data_types) | The event ID to be emitted. Event IDs are typically assigned using [`mapper.register_event()`](/libs/mapper/mapper_register_event) in Lua scripts. |

## Return Values

This function does not return a value.

## Remarks

- This function may be called from **any thread** and does not require access to the Lua stack.
- The emitted event carries no associated value.
- The event is processed by fsmapper’s event-action mapping mechanism. The event ID must correspond to an event registered in the Lua environment, either by the script or by the module calling `mapper.register_event()` from within Lua execution.

## See Also

- [`FSMAPPER_LUAC_CTX`](../data_types)
- [`FSMAPPER_EVENT_ID`](../data_types)
- [`fsmapper_luac_send_event_int`](./fsmapper_luac_send_event_int) function
- [`fsmapper_luac_send_event_float`](./fsmapper_luac_send_event_float) function
- [`fsmapper_luac_send_event_str`](./fsmapper_luac_send_event_str) function
- [Basic Functions](../basic)
