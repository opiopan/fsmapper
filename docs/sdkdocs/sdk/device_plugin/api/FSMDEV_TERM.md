---
id: FSMDEV_TERM
sidebar_position: 100
---

# FSMDEV_TERM callback function

The **`FSMDEV_TERM`** callback function is invoked immediately before the plugin module is unloaded and marks the end of the plugin module’s lifetime for a Lua script execution.

This callback provides the plugin with an opportunity to release module-level resources that were allocated during the corresponding [`FSMDEV_INIT`](FSMDEV_INIT) callback.

## Syntax

```c
typedef bool (*FSMDEV_TERM)(
    FSMAPPER_HANDLE mapper
);
````

## Parameters

|Parameter|Type| Description|
|--|--|--|
|mapper|[`FSMAPPER_HANDLE`](../data_types)| A handle representing the fsmapper runtime instance associated with the current Lua script execution.

## Return Values

Returns `true` if module-level termination completed successfully.

Returns `false` if an error occurred during termination. The plugin module will still be unloaded, but fsmapper may report the failure for diagnostic purposes.

## Remarks

* The `FSMDEV_TERM` callback is invoked once each time the plugin module is unloaded.
  Plugin modules are unloaded per Lua script execution, even if fsmapper itself continues running.

* This callback is guaranteed to be invoked after the corresponding [`FSMDEV_INIT`](FSMDEV_INIT) callback and before the plugin module is unloaded.

* The `mapper` parameter is the same opaque handle that was provided to [`FSMDEV_INIT`](FSMDEV_INIT).
  Any plugin-specific context previously associated with this handle using fsmapper runtime service functions should be retrieved and released at this point.

* Plugins should release all module-level dynamic resources allocated during [`FSMDEV_INIT`](FSMDEV_INIT), including memory, threads, and other OS or runtime resources.

* After this callback returns, the plugin module must not assume that any fsmapper runtime services remain available.

## See Also

* [Plugin Callback Execution Flow](../plugin_abi#flow)
* [`MAPPER_PLUGIN_DEVICE_OPS` structure](MAPPER_PLUGIN_DEVICE_OPS)
* [`FSMAPPER_HANDLE`](../data_types)
* [`FSMDEV_INIT` callback function](FSMDEV_INIT)
