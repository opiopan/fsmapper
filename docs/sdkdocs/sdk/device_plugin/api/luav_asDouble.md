---
id: luav_asDouble
sidebar_position: 100
---

# luav_asDouble function

The **`luav_asDouble`** function interprets the Lua value referenced by a [`LUAVALUE`](../data_types) handle as a double-precision floating-point value.

This helper is designed for extracting numeric configuration or option values from Lua objects passed to plugin callbacks such as [`FSMDEV_OPEN`](FSMDEV_OPEN), without using the Lua C API directly.

## Syntax
```c
double luav_asDouble(
    LUAVALUE lv
);
```

## Parameters

|Parameter|Type|Description|
|--|--|--|
|lv|[`LUAVALUE`](../data_types)|A handle referencing a Lua value to be interpreted as a floating-point number.|

## Return Values

Returns a floating-point value according to the type of the referenced Lua object:

- If the value is a numeric Lua object, its numeric value is converted to `double`
- If the value is a boolean Lua object:
  - `true` is converted to `1.0`
  - `false` is converted to `0.0`
- For all other Lua object types, this function returns `0.0`

## Remarks

- This function does not modify the referenced Lua value.
- Callers are not required to check the value type explicitly using [`luav_getType`](./luav_getType) function before calling this function.
- No error is raised for unsupported or non-numeric types; they are silently converted to `0.0`.
- The conversion rules are identical to those of [`luav_asInt`](./luav_asInt) function, except for the return type.
- Lua Value Access Helper functions are **not thread-safe**; concurrent execution of functions that accept a [`LUAVALUE`](../data_types) is not allowed.
- The lifetime of a [`LUAVALUE`](../data_types) is limited to the execution of the callback function that received it; accessing it after the callback returns is prohibited.

## See Also

- [`LUAVALUE` data type](../data_types)
- [`LVTYPE` enumeration](./LVTYPE)
- [`luav_getType` function](./luav_getType)
- [`luav_asInt` function](./luav_asInt)
- [`luav_asBool` function](./luav_asBool)
- [Lua Value Access Helpers](../lua_helper)
