# fsmapper_luac_putLog function

The **`fsmapper_luac_putLog`** function outputs a log message to the fsmapper console from a Lua C module.

This function is part of the Basic Functions and is **thread-safe**, allowing log messages to be emitted from any thread, including worker threads created by the module.

## Syntax

```c
void fsmapper_luac_putLog(
    FSMAPPER_LUAC_CTX ctx,
    FSMAPPER_LOG_TYPE type,
    const char* msg
);
```

## Parameters

| Parameter | Type | Description |
| --- | --- | --- |
| ctx | [`FSMAPPER_LUAC_CTX`](../data_types) | A service context identifying the Lua C module that emits the log message. |
| type | [`FSMAPPER_LOG_TYPE`](../../device_plugin/api/FSMAPPER_LOG_TYPE) | The log message category or severity, such as error, warning, or informational messages. |
| msg | `const char*` | A null-terminated UTF-8 string containing the log message to be output. |

## Return Values

This function does not return a value.

## Remarks

- This function may be called from **any thread** and does not require access to the Lua stack.
- Log messages are internally tagged with the service context, allowing fsmapper to identify the originating Lua C module.
- The `msg` string is consumed during the function call; the caller may safely release or reuse the memory after the function returns.
- This function is intended for diagnostic and informational output from native code, including asynchronous processing paths.

## See Also

- [`FSMAPPER_LUAC_CTX`](../data_types)
- [`FSMAPPER_LOG_TYPE`](../data_types) enumeration
- [`fsmapper_luac_open_ctx`](./fsmapper_luac_open_ctx) function
- [Basic Functions](../basic)
