---
id: fsmapper_getContextForDevice
sidebar_position: 100
---

# fsmapper_getContextForDevice function

The **`fsmapper_getContextForDevice`** function is used to retrieve the plugin-defined device-level context associated with a specific device instance.

This context is typically set during device initialization using the [`fsmapper_setContextForDevice`](fsmapper_setContextForDevice) function and allows subsequent callbacks to access per-device state.

## Syntax
```c
void* fsmapper_getContextForDevice(
    FSMAPPER_HANDLE mapper,
    FSMDEVICE device
);
```

## Parameters

|Parameter|Type|Description|
|--|--|--|
|`mapper`|[`FSMAPPER_HANDLE`](../data_types)|A handle representing the fsmapper runtime environment associated with the current Lua script execution.|
|`device`|[`FSMDEVICE`](../data_types)|A handle representing a device instance created by [`mapper.device()`](/libs/mapper/mapper_device) in Lua.|

## Return Values

Returns the device-level context pointer previously associated with the specified device by [`fsmapper_setContextForDevice`](fsmapper_setContextForDevice).

If no context has been set for the device, this function returns a null pointer.

## Remarks

- The returned pointer is owned and managed by the plugin.
- fsmapper does not interpret, modify, or free the context pointer.
- This function is thread-safe and may be called from threads other than the one invoking plugin callbacks.
- The returned context must not be accessed after the device has been closed via the [`FSMDEV_CLOSE`](FSMDEV_CLOSE) callback.

## See Also

- [`fsmapper_setContextForDevice` function](fsmapper_setContextForDevice)
- [`FSMDEVICE`](../data_types)
