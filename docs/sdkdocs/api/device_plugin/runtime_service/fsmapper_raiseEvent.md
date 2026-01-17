---
id: fsmapper_raiseEvent
sidebar_position: 100
---

# fsmapper_raiseEvent function

The **`fsmapper_raiseEvent`** function is used to notify fsmapper of a state change in an INPUT-type device unit.

This function allows a plugin to raise asynchronous events originating from the device, which are then propagated to Lua scripts through fsmapper’s event handling mechanism.

## Syntax
```c
void fsmapper_raiseEvent(
    FSMAPPER_HANDLE mapper,
    FSMDEVICE device,
    int index,
    int value
);
```

## Parameters

|Parameter|Type|Description|
|--|--|--|
|`mapper`|[`FSMAPPER_HANDLE`](../data_types)|A handle representing the fsmapper runtime environment associated with the current Lua script execution.|
|`device`|[`FSMDEVICE`](../data_types)|A handle representing the device instance that owns the device unit.|
|`index`|`int`|The zero-based index of the device unit whose state has changed.|
|`value`|`int`|The new value of the device unit.|

## Return Values

This function does not return a value.

## Remarks

- This function reports device unit value changes. The generation and timing of Lua-visible events are determined by [event modifier](/guide/device/#event-modifier) settings.
- This function must be used only for device units whose direction is defined as [`FSMDU_DIR_INPUT`](../plugin_abi/FSMDEVUNIT_DIRECTION.md).
- The device unit index corresponds to the order defined by successive calls to [`FSMDEV_GET_UNIT_DEF`](../plugin_abi/FSMDEV_GET_UNIT_DEF).
- This function may be called asynchronously from background threads.
- Plugins must not raise events for a device before the [`FSMDEV_START`](../plugin_abi/FSMDEV_START) callback has been invoked.
- Plugins must not raise events after the device has been closed via the [`FSMDEV_CLOSE`](../plugin_abi/FSMDEV_CLOSE) callback, as the device handle becomes invalid.

## See Also

- [`FSMDEV_GET_UNIT_DEF` callback function](../plugin_abi/FSMDEV_GET_UNIT_DEF)
- [`FSMDEV_START` callback function](../plugin_abi/FSMDEV_START)
- [`FSMDEV_CLOSE` callback function](../plugin_abi/FSMDEV_CLOSE)
- [`FSMDEVUNITDEF` structure](../plugin_abi/FSMDEVUNITDEF)
- [`FSMDEVICE`](../data_types)
