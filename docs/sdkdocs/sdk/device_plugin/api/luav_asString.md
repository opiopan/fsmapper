---
id: luav_asString
sidebar_position: 100
---

# luav_asString function

The **`luav_asString`** function retrieves the Lua string value referenced by a [`LUAVALUE`](../data_types) handle as a C-style string.

This helper is intended for reading string-based configuration or identifier values passed to plugin callbacks such as [`FSMDEV_OPEN`](FSMDEV_OPEN), without using the Lua C API directly.

## Syntax
```c
const char* luav_asString(
    LUAVALUE lv
);
```

## Parameters

|Parameter|Type|Description|
|--|--|--|
|lv|[`LUAVALUE`](../data_types)|A handle referencing a Lua value to be interpreted as a string.|

## Return Values

- If the referenced Lua value is a string, returns a pointer to a null-terminated UTF-8 string.
- If the referenced Lua value is **not** a string, returns a **null pointer**.

## Remarks

- This function does not perform type conversion; only Lua string values are accepted.
- Callers should check the return value for a null pointer before using the returned string.
- The returned string pointer is owned by the Lua runtime and must not be modified or freed by the plugin.
- The lifetime of the returned string is limited to the execution of the callback function that received the [`LUAVALUE`](../data_types) handle.
- Lua Value Access Helper functions are **not thread-safe**; concurrent execution of functions that accept a [`LUAVALUE`](../data_types) is not allowed.
- The lifetime of a [`LUAVALUE`](../data_types) is limited to the execution of the callback function that received it; accessing it after the callback returns is prohibited.

## See Also

- [`LUAVALUE` data type](../data_types)
- [`LVTYPE` enumeration](./LVTYPE)
- [`luav_getType` function](./luav_getType)
- [`FSMDEV_OPEN` callback function](FSMDEV_OPEN)
- [Lua Value Access Helpers](../lua_helper)
