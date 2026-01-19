---
id: luav_isNull
sidebar_position: 100
---

# luav_isNull function

The **`luav_isNull`** function checks whether a [`LUAVALUE`](../data_types) data type represents a Lua `nil` value.

This function is primarily intended to safely detect the absence of a value when interpreting the `identifier` or `options` parameters passed to the [`FSMDEV_OPEN`](FSMDEV_OPEN) callback function.

## Syntax
```c
bool luav_isNull(
    LUAVALUE lv
);
```

## Parameters

|Parameter|Type|Description|
|--|--|--|
|lv|[`LUAVALUE`](../data_types) data type|A handle representing a Lua value passed from fsmapper.|

## Return Values

|Value|Description|
|--|--|
|`true`|The Lua value is `nil`.|
|`false`|The Lua value is not `nil`.|

## Remarks

- This function only checks whether the Lua value is `nil`; it does not validate or convert the underlying value.
- To determine the actual Lua type of a non-`nil` value, use the [`luav_getType`](./luav_getType) function.
- Lua Value Access Helper functions, including this function, are **not thread-safe**.
- The returned result must only be used while the [`FSMDEV_OPEN`](FSMDEV_OPEN) callback function is executing. Using `LUAVALUE` handles after the callback returns is not allowed.
- Lua Value Access Helper functions are **not thread-safe**; concurrent execution of functions that accept a [`LUAVALUE`](../data_types) is not allowed.
- The lifetime of a [`LUAVALUE`](../data_types) is limited to the execution of the callback function that received it; accessing it after the callback returns is prohibited.

## See Also

- [`LUAVALUE` data type](../data_types)
- [`luav_getType` function](./luav_getType)
- [`FSMDEV_OPEN` callback function](FSMDEV_OPEN)