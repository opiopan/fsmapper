---
id: FSMDEV_CLOSE
sidebar_position: 100
---

# FSMDEV_CLOSE callback function

The **`FSMDEV_CLOSE`** callback function is invoked by fsmapper to instruct the plugin to close a device instance and release all resources associated with it.

This callback is called when a device object is explicitly closed from Lua using [`Device:close()`](/libs/mapper/Device/Device-close) or when it is automatically destroyed by the Lua garbage collector.

## Syntax
```c
typedef bool (*FSMDEV_CLOSE)(
    FSMAPPER_HANDLE mapper,
    FSMDEVICE device
);
````

## Parameters

| Parameter | Type                               | Description                                             |
| --------- | ---------------------------------- | ------------------------------------------------------- |
| mapper    | [`FSMAPPER_HANDLE`](../data_types) | A handle representing the fsmapper runtime environment. |
| device    | [`FSMDEVICE`](../data_types)       | A handle representing the device instance to be closed. |

## Return Values

Returns `true` if the device was successfully closed and all associated resources were released.

Returns `false` if an error occurred while closing the device.
In this case, fsmapper treats the failure as a Lua error.
The plugin should report the reason for the failure using the fsmapper logging functions before returning `false`.

## Remarks

* This callback represents the end of the lifetime of a specific device instance.
* After this callback is invoked, the plugin must not raise any further asynchronous events for the device, as the [`FSMDEVICE`](../data_types) handle becomes invalid and must no longer be used.
* All device-specific resources associated with the `device` handle should be released in this callback.
* If a plugin has associated a custom context with the device using [`fsmapper_setContextForDevice`](../runtime_service/fsmapper_setContextForDevice), it should be cleaned up here.
* This callback may be invoked even if device startup was only partially completed.

## See Also

* [Plugin Callback Execution Flow](.#flow)
* [`MAPPER_PLUGIN_DEVICE_OPS` structure](MAPPER_PLUGIN_DEVICE_OPS)
* [`FSMDEVICE`](../data_types)
* [`FSMDEV_OPEN` callback function](FSMDEV_OPEN)
* [`FSMDEV_START` callback function](FSMDEV_START)
* [`FSMDEV_TERM` callback function](FSMDEV_TERM)

