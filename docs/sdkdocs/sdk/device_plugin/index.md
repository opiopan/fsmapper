---
id: device_api_index
sidebar_position: 1
---

# Custom Device Plugin

This section provides a comprehensive reference for developers who want to implement Custom Device Plugin modules for fsmapper.

It is intended for readers who are already familiar with fsmapper usage and have experience with C or C++ development.
Rather than a step-by-step tutorial, this documentation focuses on the interfaces, conventions, and design concepts required to extend fsmapper with custom device implementations.

## What Is a Custom Device Plugin?

In fsmapper, a *device* is an abstract representation of an input/output source exposed to Lua scripts through [`mapper.device()`](/libs/mapper/mapper_device).

A Custom Device Plugin is a native module that implements the behavior of a specific device type within this abstraction.
By implementing such a module, fsmapper can be extended to support physical devices, virtual devices, or external systems that are not supported out of the box.

From the plugin’s perspective, interaction with fsmapper is centered around [**device units**](/guide/device#device-unit) and occurs in two directions:

- **Upstream (plugin → fsmapper)**  
  The plugin reports changes in device unit values to fsmapper.
  fsmapper interprets these value changes and, based on user configuration, determines which Lua-visible events should be generated and when.

- **Downstream (fsmapper → plugin)**  
  Lua scripts request changes to output-type device units.
  fsmapper delivers these requests to the plugin via callback functions, allowing the plugin to apply the requested state changes.

The plugin itself does not generate Lua events directly.
Instead, it focuses on reporting and applying device unit state changes, while fsmapper handles event interpretation and delivery.

For a conceptual overview of how devices are modeled and used in fsmapper, see the [Device Handling](/guide/device) section

This reference documents all interfaces and conventions required to implement Custom Device Plugin modules, including the plugin ABI, runtime services provided by fsmapper, and helper APIs for interpreting Lua-provided configuration data.

## Implementing Plugin Modules

The Custom Device Plugin API is defined using a **C-compatible binary interface**.
Both the plugin ABI and the runtime services provided by fsmapper are exposed using C linkage.

As a result, plugin modules are typically implemented in C or C++.
This design prioritizes binary compatibility and long-term stability.

While the reference documentation assumes C or C++ usage, it does not intentionally exclude other languages.
Any language capable of interoperating with a C-compatible binary interface may be used.

## Required Headers and Libraries

To implement a Custom Device Plugin module, the following files are required:

- **Header file**  
  `mapperplugin.h`  
  Defines the plugin ABI, callback function types, and related data structures.

  ```c title='C/C++'
  #incude mapperplugin.h
  ```

- **Library file**  
  `fsmappercore.lib`  
  Provides access to fsmapper runtime services used by plugin modules.   
  When using MSVC, the library can be linked either via project settings or explicitly using a pragma directive:

	```c title='C/C++'
	#pragma comment(lib, "fsmappercore.lib")
	```

## Plugin Module Placement

Custom Device Plugin modules are distributed as dynamic libraries (DLLs).

By default, fsmapper scans the following locations for plugin modules:

- The `plugins` directory under the fsmapper installation folder
- The `AppData\Roaming\fsmapper\plugins` directory under the user’s folder

Additional plugin directories can be specified via the fsmapper Settings Page.
Placing a plugin module in any of these locations allows fsmapper to discover and load it automatically.

## How to Read This Reference

This reference is organized to guide plugin developers from required specifications to supporting utilities.

The recommended reading order is:

1. [**Plugin ABI**](./plugin_abi)  
   Defines the mandatory entry point, callback functions, and execution flow that determine whether a module is recognized as a Custom Device Plugin.

2. [**fsmapper Runtime Service**](./runtime_service)  
   Describes the runtime APIs provided by fsmapper for plugin modules, including logging, context management, script control, and device unit state notification.

3. [**Lua Value Access Helper**](./lua_helper)  
   Explains helper functions for interpreting Lua objects passed to plugins—most notably the `identifier` and `options` parameters received by device open callbacks—without directly using the Lua C API.

Together, these sections provide all information required to design, implement, and integrate a Custom Device Plugin module into fsmapper.
