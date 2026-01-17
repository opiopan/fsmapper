---
id: FSMDEV_SEND_UNIT_VALUE
sidebar_position: 100
---

# FSMDEV_SEND_UNIT_VALUE callback function

The **`FSMDEV_SEND_UNIT_VALUE`** callback function is invoked by fsmapper to notify the plugin of a new value for an output device unit.

This callback is invoked when a Lua script calls [`Device:send()`](/libs/mapper/Device/Device-send) to update the value of a device unit, and allows the plugin to apply the change to the underlying device.


## Syntax
```c
typedef bool (*FSMDEV_SEND_UNIT_VALUE)(
    FSMAPPER_HANDLE mapper,
    FSMDEVICE device,
    size_t index,
    int value
);
````

## Parameters

| Parameter | Type                               | Description                                                           |
| --------- | ---------------------------------- | --------------------------------------------------------------------- |
| mapper    | [`FSMAPPER_HANDLE`](../data_types) | A handle representing the fsmapper runtime environment.               |
| device    | [`FSMDEVICE`](../data_types)       | A handle representing the target device instance.                     |
| index     | size_t                             | The zero-based index of the device unit whose value is being updated. |
| value     | int                                | The new value for the specified device unit.                          |

## Return Values

Returns `true` if the value was successfully accepted and applied by the plugin.

Returns `false` if the value could not be processed.
In this case, fsmapper treats the failure as a Lua error.
The plugin should report the reason for the failure using the fsmapper logging functions before returning `false`.

## Remarks

* This callback is only invoked for device units whose direction is [`FSMDU_DIR_OUTPUT`](FSMDEVUNIT_DIRECTION#enumerators).
* The `index` parameter corresponds to the device unit order defined via the [`FSMDEV_GET_UNIT_DEF`](FSMDEV_GET_UNIT_DEF) callback.
* The `value` parameter follows the constraints defined by the corresponding [`FSMDEVUNITDEF`](FSMDEVUNITDEF), including value type and valid range.
* This callback is invoked only after the device has been successfully started via [`FSMDEV_START`](FSMDEV_START).
* The plugin should apply the value change to the underlying device or internal state associated with the device unit.
* The `device` handle must remain valid for the duration of this callback and must not be used after the device has been closed.

## See Also

* [Plugin Callback Execution Flow](.#flow)
* [`MAPPER_PLUGIN_DEVICE_OPS` structure](MAPPER_PLUGIN_DEVICE_OPS)
* [`FSMDEVICE`](../data_types)
* [`FSMDEVUNITDEF` structure](FSMDEVUNITDEF)
* [`FSMDEV_START` callback function](FSMDEV_START)
* [`FSMDEV_CLOSE` callback function](FSMDEV_CLOSE)

