---
id: device_api_index
sidebar_position: 1
---

# Custom Device Plugin
This section documents the Custom Device Plugin API provided by fsmapper.
It is intended for developers implementing native plugin modules that extend fsmapper with custom devices and integrate them with Lua scripts.

The API is organized into three categories, each addressing a different aspect of the interaction between fsmapper, plugin modules, and Lua scripts.
Together, they form a layered interface ranging from low-level binary contracts to high-level helper utilities.

## Headers and Libraries

To use the Custom Device Plugin API, plugin modules must include the following header file and link against the corresponding library provided by fsmapper.

### Required Header
```c title="C / C++"
#include <mapperplugin.h>
```

This header declares all public types, constants, and function interfaces required to implement a custom device plugin.

### Required Library

When building a plugin module, link against the following library:
- fsmappercore.lib

On Microsoft Visual C++, the library can also be specified directly in source code using a pragma directive:

```c title="C / C++"
#pragma comment(lib, "fsmappercore.lib")
```

## Plugin ABI
The Plugin ABI defines the binary interface between fsmapper and a custom device plugin module.

A plugin module exposes an entry point function that returns a pointer to a structure containing function pointers. These functions are invoked by fsmapper in response to device operations initiated from Lua scripts, such as opening, closing, or updating a device.

This layer is intentionally low-level and language-agnostic, making it suitable for implementation in C or C++ and stable across compiler and runtime boundaries.

Use this section when you want to:
- Implement a new custom device plugin module
- Understand the lifecycle and callbacks of a device
- Verify ABI compatibility requirements

## fsmapper Runtime Services

The fsmapper Runtime Services API provides functions that allow a plugin module to interact with the fsmapper runtime environment.

These services include facilities such as logging, message output, event notification, and asynchronous interaction with the host.
They are callable from within plugin callbacks and serve as the primary communication channel from the plugin back to fsmapper.

This layer abstracts the internal mechanisms of fsmapper while offering enough control for advanced device behavior.

Use this section when you want to:
	•	Output diagnostic or informational messages
	•	Emit events or notifications to fsmapper
	•	Interact with the runtime from within plugin code

## Lua Value Access Helpers

The Lua Value Access Helpers provide a high-level API for interpreting Lua values passed to a plugin module, typically as device options specified at device open time.

These helpers are designed for developers who are not familiar with the Lua C API or its stack-based programming model.
They allow plugin code to access Lua tables, objects, and primitive values safely and consistently, without directly manipulating the Lua stack or lua_State.

This layer improves readability, reduces boilerplate code, and helps avoid common errors when handling Lua values in native code.

Use this section when you want to:
	•	Read device options specified from Lua scripts
	•	Access Lua tables and objects in a type-safe manner
	•	Avoid direct use of the Lua C API

## How to Read This Reference

If you are new to fsmapper plugin development, it is recommended to start with Plugin ABI to understand the overall structure and lifecycle of a plugin module.
Then, refer to fsmapper Runtime Services to learn how plugins communicate with the host.
Finally, consult Lua Value Access Helpers when implementing option parsing or configuration handling.

Each API entry is documented with its purpose, parameters, return values, and usage notes where applicable.