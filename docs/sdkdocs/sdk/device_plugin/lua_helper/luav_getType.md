---
id: luav_getType
sidebar_position: 100
---

# luav_getType function

The **`luav_getType`** function returns the type of the Lua value referenced by a [`LUAVALUE`](../data_types) handle.

This function allows a plugin to determine how the referenced Lua object should be interpreted before attempting to access its value.

## Syntax
```c
LVTYPE luav_getType(
    LUAVALUE lv
);
```

## Parameters

|Parameter|Type|Description|
|--|--|--|
|lv|[`LUAVALUE`](../data_types)|A handle that references a Lua value passed to the plugin, typically from the [`FSMDEV_OPEN`](../plugin_abi/FSMDEV_OPEN) callback.|

## Return Values

Returns a value of the [`LVTYPE` enumeration](LVTYPE), indicating the kind of Lua value referenced by `lv`.

## Remarks

- The returned type should be checked before calling type-specific accessors such as [`luav_asBool`](luav_asBool) or [`luav_asString`](luav_asString).
- If `lv` represents a Lua `nil` value, the function returns `LV_NULL`.
- Lua Value Access Helper functions are **not thread-safe**; concurrent execution of functions that accept a [`LUAVALUE`](../data_types) is not allowed.
- The lifetime of a [`LUAVALUE`](../data_types) is limited to the execution of the callback function that received it; accessing it after the callback returns is prohibited.

## See Also

- [`LVTYPE` enumeration](LVTYPE)
- [`LUAVALUE` data type](../data_types)
- [`luav_isNull` function](luav_isNull)
- [Lua Value Access Helper](.)
