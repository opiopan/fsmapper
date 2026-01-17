---
id: fsmapper_getContext
sidebar_position: 100
---

# fsmapper_getContext function

The **`fsmapper_getContext`** function is used to retrieve the plugin-defined module-level context associated with the specified fsmapper runtime instance.

This context pointer is typically set during module initialization (for example, in the `FSMDEV_INIT` callback) using the [`fsmapper_setContext`](fsmapper_setContext) function, and can be accessed from any callback or thread-safe runtime service call.

## Syntax
```c
void *fsmapper_getContext(FSMAPPER_HANDLE mapper);
```

## Parameters

|Parameter|Type|Description|
|--|--|--|
|`mapper`|[`FSMAPPER_HANDLE`](../data_types)|A handle representing the fsmapper runtime environment associated with the currently executing Lua script.|

## Return Values

Returns the module-level context pointer previously associated with the specified fsmapper runtime by [`fsmapper_setContext`](fsmapper_setContext).

If no context has been set, this function returns a null pointer.

## Remarks

- The returned pointer is owned and managed by the plugin.
- fsmapper does not interpret, modify, or free the context pointer.
- This function is thread-safe and may be called from threads other than the one invoking plugin callbacks.
- The lifetime of the returned context is defined by the plugin implementation and should match the lifetime of the corresponding fsmapper runtime instance.

## See Also

- [`fsmapper_setContext` function](fsmapper_setContext)
- [`FSMAPPER_HANDLE`](../data_types)
