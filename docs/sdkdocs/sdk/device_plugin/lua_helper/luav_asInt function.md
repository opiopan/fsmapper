---
id: luav_asInt
sidebar_position: 100
---

# luav_asInt function

The **`luav_asInt`** function interprets the Lua value referenced by a [`LUAVALUE`](../data_types) handle as a signed integer value.

This helper is designed for extracting integer-style configuration or option values from Lua objects passed to plugin callbacks such as [`FSMDEV_OPEN`](../plugin_abi/FSMDEV_OPEN), without using the Lua C API directly.

## Syntax
```c
int64_t luav_asInt(
    LUAVALUE lv
);
```

## Parameters

| Parameter | Type | Description |
| --- | --- | --- |
| lv | [`LUAVALUE`](../data_types) | A handle referencing a Lua value to be interpreted as an integer. |

## Return Values

Returns an integer value according to the type of the referenced Lua object:

- If the value is a numeric Lua object, its numeric value is converted to `int64_t`
- If the value is a boolean Lua object:
  - `true` is converted to `1`
  - `false` is converted to `0`
- For all other Lua object types, this function returns `0`

## Remarks

- This function does not modify the referenced Lua value.
- Callers are not required to check the value type explicitly using [`luav_getType`](./luav_getType) before calling this function.
- No error is raised for unsupported or non-numeric types; they are silently converted to `0`.
- This behavior is intentional and suitable for optional parameters and default-value handling in plugin implementations.
- Lua Value Access Helper functions are **not thread-safe**; concurrent execution of functions that accept a [`LUAVALUE`](../data_types) is not allowed.
- The lifetime of a [`LUAVALUE`](../data_types) is limited to the execution of the callback function that received it; accessing it after the callback returns is prohibited.

## See Also

- [`LUAVALUE`](../data_types)
- [`LVTYPE`](./LVTYPE) enumeration
- [`luav_getType`](./luav_getType) function
- [`luav_asBool`](./luav_asBool) function
- [`luav_asDouble`](./luav_asDouble) function
- [Lua Value Access Helpers](.)