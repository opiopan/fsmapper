---
id: fsmapper_putLog
sidebar_position: 100
---

# fsmapper_putLog function

The **`fsmapper_putLog`** function is used to output a log message from a plugin module to the fsmapper console.

This function allows plugins to report errors, warnings, informational messages, and debug output during callback execution or from background threads.

## Syntax
```c
void fsmapper_putLog(
    FSMAPPER_HANDLE mapper,
    FSMAPPER_LOG_TYPE type,
    const char* msg
);
```

## Parameters

|Parameter|Type|Description|
|--|--|--|
|`mapper`|[`FSMAPPER_HANDLE`](../data_types)|A handle representing the fsmapper runtime environment associated with the current Lua script execution.|
|`type`|[`FSMAPPER_LOG_TYPE`](FSMAPPER_LOG_TYPE)|The severity level of the log message.|
|`msg`|`const char*`|A null-terminated UTF-8 string containing the log message text.|

## Return Values

This function does not return a value.

## Remarks

- This function is thread-safe and may be called from threads other than the one invoking plugin callbacks.
- Log messages are typically displayed in the fsmapper console and may also be recorded depending on fsmapper configuration.
- When a plugin callback returns a failure result that causes a Lua error, the error reason should be reported using this function before returning.
- The message string must remain valid for the duration of the function call only.

## See Also

- [`FSMAPPER_LOG_TYPE` enumeration](FSMAPPER_LOG_TYPE)
- [`FSMAPPER_HANDLE`](../data_types)
- [`FSMDEV_OPEN` callback function](../plugin_abi/FSMDEV_OPEN)
