---
id: FSMDEV_OPEN
sidebar_position: 100
---

# FSMDEV_OPEN callback function

The **`FSMDEV_OPEN`** callback function is invoked when a Lua script creates a new device instance of the plugin-defined device type using [`mapper.device()`](/libs/mapper/mapper_device).

This callback marks the beginning of a device instance’s lifetime and provides an opportunity for the plugin to perform device-level initialization and resource allocation.

## Syntax

```c
typedef bool (*FSMDEV_OPEN)(
    FSMAPPER_HANDLE mapper,
    FSMDEVICE device,
    LUAVALUE identifier,
    LUAVALUE options
);
````

## Parameters

| Parameter | Type | Description
|--|--|--|
| mapper| [`FSMAPPER_HANDLE`](../data_types)| A handle representing the fsmapper runtime instance associated with the current Lua script execution. 
| device| [`FSMDEVICE`](../data_types)| A handle representing the device instance created by [`mapper.device()`](/libs/mapper/mapper_device). This handle corresponds to the Lua `Device` object returned to the script.
| identifier| [`LUAVALUE`](../data_types)| A Lua value corresponding to the `identifier` parameter passed to [`mapper.device()`](/libs/mapper/mapper_device).
| options| [`LUAVALUE`](../data_types)| A Lua value corresponding to the `options` parameter passed to [`mapper.device()`](/libs/mapper/mapper_device).

## Return Values

Returns `true` if the device instance was successfully opened and initialized.

Returns `false` if device initialization failed.
In this case, fsmapper raises a Lua error from the [`mapper.device()`](/libs/mapper/mapper_device) call.
Unless the error is explicitly handled by the Lua script, script execution will be aborted.

Because the error originates in the Lua execution context, the plugin implementation **must**
output the reason for the failure to the console using [`fsmapper_putLog`](../runtime_service/fsmapper_putLog) function before returning `false`.

## Remarks

* The `FSMDEV_OPEN` callback is invoked once for each call to [`mapper.device()`](/libs/mapper/mapper_device) that specifies the corresponding device type.

* The `device` parameter is an opaque handle representing a single device instance.
  Plugin implementations may associate plugin-specific, device-level context data with this handle by using the fsmapper runtime service function [`fsmapper_setContextForDevice`](../runtime_service/fsmapper_setContextForDevice).

* Device-level context data associated with the [`FSMDEVICE`](../data_types) handle can later be retrieved in other device-related callbacks using [`fsmapper_getContextForDevice`](../runtime_service/fsmapper_getContextForDevice).
  This mechanism is intended for managing resources that are scoped to a single device instance.

* The [`FSMDEVICE`](../data_types) handle is also used when raising device unit state change events through [fsmapper runtime services](../runtime_service/).
  Plugins are therefore expected to retain this handle as part of the device-specific context.

* The `identifier` and `options` parameters reference Lua objects passed to `mapper.device()` by the script.
  These values are opaque to the plugin and must be accessed through the [Lua Value Access Helper APIs](../lua_helper/).

* Plugins should not raise device unit state change events before the corresponding [`FSMDEV_START`](FSMDEV_START) callback has been invoked.

## See Also

* [Plugin Callback Execution Flow](.#flow)
* [`MAPPER_PLUGIN_DEVICE_OPS` structure](MAPPER_PLUGIN_DEVICE_OPS)
* [`FSMDEVICE`](../data_types)
* [`LUAVALUE`](../data_types)
* [`FSMDEV_START` callback function](FSMDEV_START)
* [`FSMDEV_CLOSE` callback function](FSMDEV_CLOSE)

