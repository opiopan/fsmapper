---
id: data_types
sidebar_position: 1000
---

# Data Types
This page documents the type aliases shared across all categories of the fsmapper Custom Device Plugin API.
Many of these types are defined as pointers to opaque structures, providing strong type safety while intentionally hiding internal implementation details.

| Type Name | C Representation | Description |
|--|--|--|
| `FSMAPPER_HANDLE` | `struct FSMAPPERCTX*` | A handle representing the fsmapper core context. This value is provided by fsmapper and passed to plugin callbacks to allow interaction with the runtime, such as reporting events or accessing shared services. Plugin implementations must treat this handle as opaque and must not dereference it directly. |
| `FSMDEVICE` | `struct FSMDEVICECTX*` | A handle representing a device instance created by fsmapper via the plugin. This handle is passed to device-related callback functions and identifies the logical device managed by the plugin. The internal structure is owned by fsmapper and must not be accessed directly by the plugin. |
| `LUAVALUE` | `struct LUAVALUECTX*` | An opaque handle representing a Lua value passed between fsmapper and the plugin. This type is used to safely exchange values originating from Lua scripts without exposing Lua VM internals or stack state to the plugin implementation. |
