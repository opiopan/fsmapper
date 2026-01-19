---
id: luav_getItemWithKey
sidebar_position: 100
---

# luav_getItemWithKey function

The **`luav_getItemWithKey`** function retrieves a value from a Lua table referenced by a [`LUAVALUE`](../data_types) handle, using a string key.

This helper is primarily intended for accessing fields of the `identifier` or `options` table passed to the [`FSMDEV_OPEN`](FSMDEV_OPEN) callback function.

## Syntax
```c
LUAVALUE luav_getItemWithKey(
    LUAVALUE lv,
    const char* key
);
```

## Parameters

|Parameter|Type|Description|
|--|--|--|
|lv|[`LUAVALUE`](../data_types)|A handle referencing a Lua value expected to be a table.|
|key|`const char*`|A null-terminated UTF-8 string specifying the table key to access.|

## Return Values

- If `lv` refers to a Lua table and the specified key exists, returns a `LUAVALUE` referencing the corresponding table element.
- If the specified key does **not** exist in the table, returns a `LUAVALUE` representing Lua `nil`.
- If `lv` does not refer to a Lua table, returns a `LUAVALUE` representing Lua `nil`.

## Remarks

- This function performs a read-only table lookup and does not modify the Lua table.
- The returned `LUAVALUE` may represent any Lua value type, including `nil`.
- To test whether the returned value is `nil`, use the [`luav_isNull`](./luav_isNull) function.
- This function does not raise an error for missing keys or non-table values; such cases are represented by a `nil` Lua value.
- Lua Value Access Helper functions are **not thread-safe**; concurrent execution of functions that accept a [`LUAVALUE`](../data_types) is not allowed.
- The lifetime of a [`LUAVALUE`](../data_types) is limited to the execution of the callback function that received it; accessing it after the callback returns is prohibited.

## See Also

- [`LUAVALUE` data type](../data_types)
- [`luav_isNull` function](./luav_isNull)
- [`luav_getType` function](./luav_getType)
- [`luav_getItemWithIndex` function](./luav_getItemWithIndex)
- [`FSMDEV_OPEN` callback function](FSMDEV_OPEN)
- [Lua Value Access Helpers](../lua_helper)
