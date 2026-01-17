---
id: luav_asBool
sidebar_position: 100
---

# luav_asBool function

The **`luav_asBool`** function interprets the Lua value referenced by a [`LUAVALUE`](../data_types) handle as a boolean value.

This helper is intended for extracting boolean-style configuration values from Lua objects passed to plugin callbacks such as [`FSMDEV_OPEN`](../plugin_abi/FSMDEV_OPEN), without requiring direct use of the Lua C API.

## Syntax
```c
bool luav_asBool(
    LUAVALUE lv
);
```

## Parameters

| Parameter | Type | Description |
| --- | --- | --- |
| lv | [`LUAVALUE`](../data_types) | A handle referencing a Lua value to be interpreted as a boolean. |

## Return Values

Returns `true` if the referenced Lua value satisfies **any** of the following conditions:

- The value is a boolean Lua object and its value is `true`
- The value is a numeric Lua object and its value is non-zero
- The value is a string Lua object and its length is one or more characters

Returns `false` in all other cases.

## Remarks

- This function performs a value interpretation only and does not modify the referenced Lua object.
- Callers are not required to inspect the value type explicitly using [`luav_getType`](./luav_getType) before calling this function.
- Values that are `nil`, unsupported, or not interpretable as boolean semantics result in `false`.
- The interpretation rules are intentionally designed for option and configuration handling rather than full Lua language semantics.
- Lua Value Access Helper functions are **not thread-safe**; concurrent execution of functions that accept a [`LUAVALUE`](../data_types) is not allowed.
- The lifetime of a [`LUAVALUE`](../data_types) is limited to the execution of the callback function that received it; accessing it after the callback returns is prohibited.

## See Also

- [`LUAVALUE`](../data_types)
- [`LVTYPE`](./LVTYPE) enumeration
- [`luav_getType`](./luav_getType) function
- [`luav_isNull`](./luav_isNull) function
- [Lua Value Access Helpers](.)