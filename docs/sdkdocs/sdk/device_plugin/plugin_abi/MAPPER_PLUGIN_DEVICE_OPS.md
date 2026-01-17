---
id: MAPPER_PLUGIN_DEVICE_OPS
sidebar_position: 100
---

# MAPPER_PLUGIN_DEVICE_OPS structure

The **`MAPPER_PLUGIN_DEVICE_OPS`** structure defines the complete set of callback functions and metadata that a Custom Device Plugin exposes to fsmapper.

An instance of this structure is returned by the plugin entry point function [`getMapperPluginDeviceOps`](getMapperPluginDeviceOps), and serves as the primary ABI contract between fsmapper and the plugin module. fsmapper uses this structure to identify the device type provided by the plugin and to invoke device lifecycle and data handling callbacks in response to Lua script execution.

## Syntax

```c
typedef struct {
    const char* name;
    const char* description;
    FSMDEV_INIT init;
    FSMDEV_TERM term;
    FSMDEV_OPEN open;
    FSMDEV_START start;
    FSMDEV_CLOSE close;
    FSMDEV_GET_UNIT_NUM getUnitNum;
    FSMDEV_GET_UNIT_DEF getUnitDef;
    FSMDEV_SEND_UNIT_VALUE sendUnitValue;
} MAPPER_PLUGIN_DEVICE_OPS;
````

## Members

|Member|Type|Description|
|--|--|--|
|`name`| `const char*` | A unique device type name provided by the plugin module. This name identifies the device type and must match the value specified as the `type` parameter when calling [`mapper.device()`](/libs/mapper/mapper_device) from Lua scripts. The name must be unique within the plugin module.
|`description`|`const char*`| A human-readable description of the device type. This string is intended for informational and diagnostic purposes, such as logging or UI display.
|`init`|[`FSMDEV_INIT`](FSMDEV_INIT)| Callback invoked immediately after the plugin module is loaded for a Lua script execution.<br/>This function performs module-level initialization and may be called multiple times during the lifetime of fsmapper, once for each Lua script startup.
|`term`|[`FSMDEV_TERM`](FSMDEV_TERM)| Callback invoked when the plugin module is being unloaded. This function releases module-level resources allocated during `init`.
|`open`|[`FSMDEV_OPEN`](FSMDEV_OPEN)| Callback invoked when a Lua script creates a new device instance of this type via [`mapper.device()`](/libs/mapper/mapper_device). This function performs device-level initialization and opens the device.
|`start`|[`FSMDEV_START`](FSMDEV_START)| Callback invoked after device units have been queried and the Device object has been fully constructed. This function signals the start of active device operation.
|`close`|[`FSMDEV_CLOSE`](FSMDEV_CLOSE)| Callback invoked when the device is explicitly closed from Lua or when the Device object is destroyed by garbage collection. This function releases all resources associated with the device instance.
|`getUnitNum`|[`FSMDEV_GET_UNIT_NUM`](FSMDEV_GET_UNIT_NUM)| Callback invoked to query the number of device units provided by the device instance.
|`getUnitDef`|[`FSMDEV_GET_UNIT_DEF`](FSMDEV_GET_UNIT_DEF)| Callback invoked repeatedly to retrieve the definition of each device unit. This function is called once per device unit index.
|`sendUnitValue`|[`FSMDEV_SEND_UNIT_VALUE`](FSMDEV_SEND_UNIT_VALUE)| Callback invoked when a Lua script updates the value of an output-type device unit via [`Device:send()`](/libs/mapper/Device/Device-send).

## Remarks

* All members of the `MAPPER_PLUGIN_DEVICE_OPS` structure are **mandatory**.
  Plugin implementations must provide valid function pointers for all callback members.
  Setting any callback pointer to `NULL` is not permitted.

* The `name` member defines the device type identifier as seen from Lua.
  fsmapper uses this value to associate Lua-side [`mapper.device()`](/libs/mapper/mapper_device) calls with the corresponding plugin-provided device implementation.

* The callbacks defined in this structure are invoked by fsmapper in a well-defined order that reflects the lifecycle of a device instance.
  Refer to the lifecycle sequence diagram in the [Plugin ABI](.#flow) section for details.

## See also

* [`getMapperPluginDeviceOps` function](getMapperPluginDeviceOps)
* [Plugin ABI](.)
