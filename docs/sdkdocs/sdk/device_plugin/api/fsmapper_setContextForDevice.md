---
id: fsmapper_setContextForDevice
sidebar_position: 100
---

# fsmapper_setContextForDevice function

The **`fsmapper_setContextForDevice`** function is used to associate a plugin-defined device-level context with a specific device instance.

This context is typically allocated during the [`FSMDEV_OPEN`](FSMDEV_OPEN) callback and allows the plugin to store per-device state that can be retrieved later from other callbacks or runtime service functions.

## Syntax
```c
void fsmapper_setContextForDevice(
    FSMAPPER_HANDLE mapper,
    FSMDEVICE device,
    void* context
);
```

## Parameters

|Parameter|Type|Description|
|--|--|--|
|`mapper`|[`FSMAPPER_HANDLE`](../data_types)|A handle representing the fsmapper runtime environment associated with the current Lua script execution.|
|`device`|[`FSMDEVICE`](../data_types)|A handle representing a device instance created by [`mapper.device()`](/libs/mapper/mapper_device) in Lua.|
|`context`|`void*`|A plugin-defined pointer to arbitrary device-level context data.|

## Return Values

This function does not return a value.

## Remarks

- The context pointer is owned and managed entirely by the plugin.
- fsmapper does not inspect, modify, or free the context pointer.
- The associated context can be retrieved later using [`fsmapper_getContextForDevice`](fsmapper_getContextForDevice).
- This function is thread-safe and may be called from threads other than the one invoking plugin callbacks.
- The device handle becomes invalid after the [`FSMDEV_CLOSE`](FSMDEV_CLOSE) callback; the associated context should not be accessed beyond that point.

## See Also

- [`fsmapper_getContextForDevice` function](fsmapper_getContextForDevice)
- [`FSMDEVICE`](../data_types)
- [`FSMDEV_OPEN` callback function](FSMDEV_OPEN)
- [`FSMDEV_CLOSE` callback function](FSMDEV_CLOSE)
