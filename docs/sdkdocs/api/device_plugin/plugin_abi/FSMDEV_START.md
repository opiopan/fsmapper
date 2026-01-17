---
id: FSMDEV_START
sidebar_position: 100
---

# FSMDEV_START callback function

The **`FSMDEV_START`** callback function is invoked by fsmapper to instruct the plugin to start device processing.

This callback marks the point at which the plugin is allowed to raise asynchronous events for input device units associated with the device.

## Syntax
```c
typedef bool (*FSMDEV_START)(
    FSMAPPER_HANDLE mapper,
    FSMDEVICE device
);
````

## Parameters

| Parameter | Type                               | Description                                              |
| --------- | ---------------------------------- | -------------------------------------------------------- |
| mapper    | [`FSMAPPER_HANDLE`](../data_types) | A handle representing the fsmapper runtime environment.  |
| device    | [`FSMDEVICE`](../data_types)       | A handle representing the device instance to be started. |

## Return Values

Returns `true` if the device was successfully started.

Returns `false` if device startup failed. In this case, fsmapper treats the failure as a Lua error and the device object is not made available to the script.
The plugin should report the reason for the failure using the fsmapper logging functions before returning `false`.

## Remarks

* This callback defines the boundary between device initialization and active operation.
* Before this callback is invoked, the plugin must not raise any asynchronous events for input device units.
* After this callback returns successfully, the plugin may begin raising input device unit events at any time.
* Device-specific runtime resources such as threads, timers, or I/O loops are typically started at this point.
* The `device` handle should be retained by the plugin for use when raising device unit events or handling subsequent callbacks.

## See Also

* [Plugin Callback Execution Flow](.#flow)
* [`MAPPER_PLUGIN_DEVICE_OPS` structure](MAPPER_PLUGIN_DEVICE_OPS)
* [`FSMDEVICE`](../data_types)
* [`FSMDEV_OPEN` callback function](FSMDEV_OPEN)
* [`FSMDEV_SEND_UNIT_VALUE` callback function](FSMDEV_SEND_UNIT_VALUE)

