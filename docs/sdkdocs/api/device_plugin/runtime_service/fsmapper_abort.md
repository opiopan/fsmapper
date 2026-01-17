---
id: fsmapper_abort
sidebar_position: 100
---

# fsmapper_abort function

The **`fsmapper_abort`** function is used to request immediate termination of the currently running Lua script.

This function allows a plugin to abort script execution when a fatal error or unrecoverable condition is detected during callback execution or background processing.

## Syntax
```c
void fsmapper_abort(
    FSMAPPER_HANDLE mapper
);
```

## Parameters

|Parameter|Type|Description|
|--|--|--|
|`mapper`|[`FSMAPPER_HANDLE`](../data_types)|A handle representing the fsmapper runtime environment associated with the current Lua script execution.|

## Return Values

This function does not return a value.

## Remarks

- Calling this function causes fsmapper to terminate the current Lua script execution.
- Script termination is performed as a Lua-level error; scripts that do not handle errors will stop immediately.
- This function is thread-safe and may be called from threads other than the one invoking plugin callbacks.
- Plugins should use this function only for fatal conditions that cannot be safely handled or reported through normal error paths.
- It is recommended to report the reason for aborting execution using [`fsmapper_putLog`](fsmapper_putLog) before calling this function.

## See Also

- [`fsmapper_putLog` function](fsmapper_putLog)
- [`FSMAPPER_HANDLE`](../data_types)
- [`FSMDEV_OPEN` callback function](../plugin_abi/FSMDEV_OPEN)
