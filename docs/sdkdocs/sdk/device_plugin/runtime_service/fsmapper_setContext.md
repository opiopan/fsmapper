---
id: fsmapper_setContext
sidebar_position: 100
---

# fsmapper_setContext function

The **`fsmapper_setContext`** function is used to associate a plugin-defined context pointer with the plugin module instance.

This function allows a plugin to store arbitrary module-level state and retrieve it later through
[`fsmapper_getContext`](fsmapper_getContext), typically from other callback functions or runtime service calls.

## Syntax

```c
void fsmapper_setContext(FSMAPPER_HANDLE mapper, void *context);
````

## Parameters

| Parameter | Type | Description |
|--|--|--|
| `mapper` | [`FSMAPPER_HANDLE`](../plugin_abi/data_types) | A handle representing the fsmapper runtime environment associated with the current plugin execution. |
| `context` | `void*` | A plugin-defined pointer to arbitrary module-level context data to be associated with the mapper handle. |


## Description

This function binds a context pointer to the plugin module as a whole.
The stored context is not associated with any specific device instance and is shared across all devices created by the plugin.

The fsmapper runtime does not manage the lifetime of the context object.
Memory allocation, ownership, and cleanup are entirely the responsibility of the plugin implementation.

Calling `fsmapper_setContext` multiple times overwrites the previously stored context pointer.

## Remarks

* The context pointer is stored as-is and is never dereferenced by fsmapper.
* This function is typically called during `FSMDEV_INIT`, but it may be called at any time.
* The stored context can be retrieved later using `fsmapper_getContext`.
* The context is cleared automatically when the plugin module is unloaded.
* This function is thread-safe and may be called from any thread.

## See Also

* [`fsmapper_getContext`](fsmapper_getContext)
* [`FSMDEV_INIT` callback function](../plugin_abi/FSMDEV_INIT)
* [`FSMAPPER_HANDLE`](../data_types)
