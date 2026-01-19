---
id: FSMDEV_GET_UNIT_DEF
sidebar_position: 100
---

# FSMDEV_GET_UNIT_DEF callback function

The **`FSMDEV_GET_UNIT_DEF`** callback function is invoked to retrieve the definition of a specific device unit.

For each device unit index reported by the [`FSMDEV_GET_UNIT_NUM`](FSMDEV_GET_UNIT_NUM) callback, fsmapper calls this callback to obtain the corresponding unit definition.

## Syntax

```c
typedef bool (*FSMDEV_GET_UNIT_DEF)(
    FSMAPPER_HANDLE mapper,
    FSMDEVICE device,
    size_t index,
    FSMDEVUNITDEF *def
);
````

## Parameters

| Parameter | Type                               | Description                                                                                           |
| --------- | ---------------------------------- | ----------------------------------------------------------------------------------------------------- |
| mapper    | [`FSMAPPER_HANDLE`](../data_types) | A handle representing the fsmapper runtime instance associated with the current Lua script execution. |
| device    | [`FSMDEVICE`](../data_types)       | A handle representing the device instance to which the unit belongs.                                  |
| index     | `size_t`                           | A zero-based index identifying the device unit whose definition is being requested.                   |
| def       | [`FSMDEVUNITDEF`](FSMDEVUNITDEF)   | A pointer to a structure where the device unit definition must be written.                            |

## Return Values

Returns `true` if the device unit definition for the specified index is successfully written to `def`.

Returns `false` if the unit definition cannot be provided.
In this case, fsmapper treats the failure as a Lua error.

## Remarks

* The `index` parameter ranges from $0$ to $N - 1$, where $N$ is the value previously returned by the [`FSMDEV_GET_UNIT_NUM`](FSMDEV_GET_UNIT_NUM) callback.

* The implementation must populate the structure pointed to by `def` with valid data describing the specified device unit.

* The [`FSMDEVUNITDEF`](FSMDEVUNITDEF) structure defines the characteristics of a device unit, such as its name, direction, value type, and valid value range.
  The detailed structure layout and semantics are described on a [separate reference page](FSMDEVUNITDEF).

* The set of device unit definitions returned by this callback must be consistent for the lifetime of the device instance.

* If this callback returns `false`, the [`mapper.device()`](/libs/mapper/mapper_device) call in Lua results in a Lua error.
  In such cases, the plugin should report the cause of the failure using the fsmapper runtime logging service (for example, `fsmapper_putLog`) before returning `false`.

* The `device` parameter may be used to access device-specific context data previously associated with the device instance.

## See Also

* [Plugin Callback Execution Flow](../plugin_abi.md#flow)
* [`MAPPER_PLUGIN_DEVICE_OPS` structure](MAPPER_PLUGIN_DEVICE_OPS)
* [`FSMDEV_GET_UNIT_NUM` callback function](FSMDEV_GET_UNIT_NUM)
* [`FSMDEVUNITDEF` structure](FSMDEVUNITDEF)
* [Device Unit Concepts](/guide/device/#device-unit)

