---
id: FSMDEV_GET_UNIT_NUM
sidebar_position: 100
---

# FSMDEV_GET_UNIT_NUM callback function

The **`FSMDEV_GET_UNIT_NUM`** callback function is invoked to query the number of device units provided by a device instance.

fsmapper uses the returned value to determine how many times the corresponding [`FSMDEV_GET_UNIT_DEF`](FSMDEV_GET_UNIT_DEF) callback will be invoked in order to retrieve the definition of each device unit.

## Syntax

```c
typedef size_t (*FSMDEV_GET_UNIT_NUM)(
    FSMAPPER_HANDLE mapper,
    FSMDEVICE device
);
````

## Parameters

| Parameter | Type                               | Description
|--|--|--|
| mapper    | [`FSMAPPER_HANDLE`](../data_types) | A handle representing the fsmapper runtime instance associated with the current Lua script execution.
| device    | [`FSMDEVICE`](../data_types)       | A handle representing the device instance whose device units are being queried.

## Return Values

Returns the number of device units provided by the specified device instance.

A return value of `0` indicates that the device provides no units.

## Remarks

* The `FSMDEV_GET_UNIT_NUM` callback is invoked after the device instance has been successfully opened via [`FSMDEV_OPEN`](FSMDEV_OPEN).

* The returned value defines the range of unit indices that fsmapper will use when invoking [`FSMDEV_GET_UNIT_DEF`](FSMDEV_GET_UNIT_DEF).
  fsmapper will call `FSMDEV_GET_UNIT_DEF` once for each unit index from `0` to `N - 1`, where `N` is the value returned by this callback.

* The number of device units returned by this callback must remain constant for the lifetime of the device instance.

* The `device` parameter may be used to retrieve device-specific context data previously associated with the device using fsmapper runtime service functions.

## See Also

* [Plugin Callback Execution Flow](../plugin_abi#flow)
* [`MAPPER_PLUGIN_DEVICE_OPS` structure](MAPPER_PLUGIN_DEVICE_OPS)
* [`FSMDEVICE`](../data_types)
* [`FSMDEV_GET_UNIT_DEF` callback function](FSMDEV_GET_UNIT_DEF)
* [`FSMDEV_OPEN` callback function](FSMDEV_OPEN)

