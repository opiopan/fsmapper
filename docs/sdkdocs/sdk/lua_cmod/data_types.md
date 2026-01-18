---
id: data_types
sidebar_position: 300
---

# Data Types

This page documents the type aliases used by the fsmapper service API for Lua C modules.
These types are primarily defined as pointers to opaque structures, providing a safe and stable interface while hiding internal implementation details managed by fsmapper.

| Type Name | C Representation | Description |
|--|--|--|
| `FSMAPPER_LUAC_CTX` | `struct _FSMAPPER_LUAC_CTX*` | A handle representing a service context used by Lua C modules to access fsmapper-provided services. This context is required when emitting events or outputting log messages from native code. The handle is opaque and owned by fsmapper; Lua C modules must not dereference it directly and must release it explicitly when no longer needed. |
| `FSMAPPER_LUAC_ASYNC_SOURCE` | `struct _FSMAPPER_LUAC_ASYNC_SOURCE*` | A handle representing an asynchronous event source. This handle identifies a source that bridges asynchronous native code and Lua script execution, allowing Lua C modules to emit events whose values are arbitrary Lua objects. The internal structure is managed by fsmapper and must be treated as opaque by the module. |
| `FSMAPPER_EVENT_ID` | `unsigned long long` | A numeric identifier representing an fsmapper event. Event IDs are assigned by Lua function [`mapper.register_event()`](/libs/mapper/mapper_register_event) and are used by Lua C modules to emit events that trigger corresponding actions defined in the script.
