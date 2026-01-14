---
id: getMapperPluginDeviceOps
sidebar_position: 100
---

# getMapperPluginDeviceOps function

The **`getMapperPluginDeviceOps`** function is the mandatory entry point of a Custom Device Plugin and returns a pointer to the [`MAPPER_PLUGIN_DEVICE_OPS`](MAPPER_PLUGIN_DEVICE_OPS) structure that defines the device type and its callback functions.

fsmapper calls this function immediately after loading the plugin module in order to discover the device type name and obtain the complete set of callback functions that implement the device behavior.

## Syntax

```c
__declspec(dllexport) const MAPPER_PLUGIN_DEVICE_OPS* getMapperPluginDeviceOps();
````

## Parameters

This function takes no parameters.

## Return Values

The function returns a pointer to a constant [`MAPPER_PLUGIN_DEVICE_OPS`](MAPPER_PLUGIN_DEVICE_OPS) structure.

The returned pointer must remain valid for the entire time the plugin module is loaded. fsmapper does not take ownership of the structure and will not modify its contents.

Returning a null pointer is not permitted.

## Remarks

* This function must be exported from the plugin module with the exact name `getMapperPluginDeviceOps` and use **C linkage**.<br/>
    If the plugin is implemented in C++, the function must be declared with `extern "C"`; otherwise, fsmapper will not be able to locate the symbol due to C++ name mangling.

* fsmapper calls this function each time the plugin module is loaded.
  Plugin modules are loaded and unloaded per Lua script execution, not once at fsmapper startup.
  As a result, this function may be called multiple times during the lifetime of the fsmapper process.

* The structure referenced by the returned pointer is treated as read-only by fsmapper and must not be modified after it has been returned.

## See also

* [`MAPPER_PLUGIN_DEVICE_OPS` structure](MAPPER_PLUGIN_DEVICE_OPS)
* [Plugin ABI](.)
