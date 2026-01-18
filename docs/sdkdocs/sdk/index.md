---
id: sdk_index
sidebar_position: 1
---

# Plugin SDK Reference

The fsmapper Plugin SDK provides native extension mechanisms that allow developers to extend fsmapper beyond what is possible with Lua scripts alone.

This reference documentation covers the APIs and conventions required to implement plugin modules supported by fsmapper.

## Supported Plugin Types

The fsmapper Plugin SDK supports two distinct types of plugin modules:

### Custom Device Plugin

Custom Device Plugins are native modules that allow developers to define **new device types** for fsmapper.
These plugins integrate directly with fsmapper’s device model and are typically used to represent physical devices, virtual devices, or external systems as fsmapper devices.

Documentation for developing Custom Device Plugins is available here:

- [Custom Device Plugin](./device_plugin/)

### Lua C Module

Lua C Modules are native modules that extend Lua scripts executed by fsmapper.
They are loaded via Lua’s standard module mechanism and can provide additional Lua functions, userdata, and asynchronous event sources implemented in C or C++.

Documentation for developing Lua C Modules is available here:

- [Lua C Module](./lua_cmod/)

Each plugin type serves a different purpose and uses a different integration model.
For an overview of the Plugin SDK, including guidance on choosing the appropriate plugin type and installation details, see the [Plugin SDK overview](../).