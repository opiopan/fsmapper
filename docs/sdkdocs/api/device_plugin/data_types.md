---
id: data_types
sidebar_position: 2
---

# Data Types
This page documents the type aliases shared across all categories of the fsmapper Custom Device Plugin API.
Many of these types are defined as pointers to opaque structures, providing strong type safety while intentionally hiding internal implementation details.

|Type Name|C Representation| Description|
|--|--|--|
|`FSMAPPER_HANDLE`|`struct FSMAPPERCTX*`|
|`FSMAPPER_EVENT_ID`|`unsigned long long`|
|`FSMDEVICE`|`struct FSMDEVICECTX*`|
|`LUAVALUE`|`struct LUAVALUECTX*`|
