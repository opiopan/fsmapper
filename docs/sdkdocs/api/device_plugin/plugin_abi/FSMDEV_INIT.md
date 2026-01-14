---
id: FSMDEV_INIT
sidebar_position: 100
---

# FSMDEV_INIT callback function

The **`FSMDEV_INIT`** callback function is invoked immediately after the plugin module is loaded and is intended to provide an opportunity for module-level initialization for a Lua script execution.

During this callback, a handle representing the fsmapper runtime environment is provided to the plugin, which may be used to interact with fsmapper through [runtime service functions](../runtime_service/), depending on the plugin’s implementation.

## Syntax

```c
typedef bool (*FSMDEV_INIT)(
    FSMAPPER_HANDLE mapper
);
````

## Parameters

|Parameter |Type| Description|
|--|--|--|
|`mapper` |[`FSMAPPER_HANDLE`](../data_types)| A handle representing the fsmapper runtime instance associated with the current Lua script execution. This handle is passed to fsmapper runtime service functions and serves as the plugin’s primary interface to fsmapper.

## Return Values

Returns `true` if the plugin module was successfully initialized.

Returns `false` if initialization failed. In this case, fsmapper will treat the plugin as unusable for the current Lua script execution and will unload the module.

## Remarks

* The `FSMDEV_INIT` callback is invoked once each time the plugin module is loaded.
  Plugin modules are loaded and unloaded per Lua script execution, not once at fsmapper startup.
  As a result, this callback may be called multiple times during the lifetime of the fsmapper process.

* The `mapper` parameter is an opaque handle that represents the fsmapper runtime itself from the plugin’s point of view.
  The internal structure of this handle is not exposed and must not be accessed directly by the plugin.

* The `mapper` handle must be passed to [fsmapper runtime service functions](../runtime_service/) in order to perform operations such as message output, event notification, or context association.

* Plugins may associate plugin-specific context data with the [`FSMAPPER_HANDLE`](../data_types) by using the fsmapper runtime service function [`fsmapper_setContext`](../runtime_service/fsmapper_setContext).
  This callback is the recommended place to allocate and initialize module-level dynamic resources and attach them to the handle.

* Context data associated with the `FSMAPPER_HANDLE` can later be retrieved by the [`fsmapper_getContext`](../runtime_service/fsmapper_getContext) function and used by other callback functions invoked for the same plugin module, allowing shared state to be maintained across callbacks.

* Any resources allocated during `FSMDEV_INIT` should be released in the corresponding [`FSMDEV_TERM`](FSMDEV_TERM) callback.

## See Also

* [Plugin Callback Execution Flow](.#flow)
* [`MAPPER_PLUGIN_DEVICE_OPS` structure](MAPPER_PLUGIN_DEVICE_OPS)
* [`FSMAPPER_HANDLE`](../data_types)
* [`FSMDEV_TERM` callback function](FSMDEV_TERM)
