---
id: LVTYPE
sidebar_position: 100
---

# LVTYPE enumeration

The **`LVTYPE`** enumeration represents the runtime type of a Lua value referenced by a `LUAVALUE` handle.
It allows device plugins to safely inspect the kind of data passed from Lua scripts—most notably the `identifier` and `options` parameters received by the `FSMDEV_OPEN` callback—before accessing the value itself.

This enumeration is primarily used in conjunction with `luav_getType()` and `luav_isNull()` to determine how a Lua value can be interpreted from C/C++ code.

## Syntax

```c
typedef enum {
    LV_NULL,
    LV_BOOL,
    LV_NUMBER,
    LV_STRING,
    LV_TABLE,
    LV_OTHERS,
} LVTYPE;
```

## Enumeration Values

|Value|Description|
|--|--|
|`LV_NULL`|Represents a Lua `nil` value. This is typically used to indicate that a parameter was omitted or explicitly set to `nil` in Lua.|
|`LV_BOOL`|Represents a Lua boolean value (`true` or `false`).|
|`LV_NUMBER`|Represents a Lua numeric value. Depending on the access function used, it can be retrieved as an integer or a floating-point number.|
|`LV_STRING`|Represents a Lua string value. The value can be accessed as a null-terminated UTF-8 string.|
|`LV_TABLE`|Represents a Lua table. Table elements can be accessed using the Lua Value Access Helper table access functions.|
|`LV_OTHERS`|Represents any Lua value that does not fall into the categories above, such as functions, userdata, or threads. These values cannot be directly accessed as primitive C/C++ types.|

## Remarks

- The `LVTYPE` value describes the **dynamic type** of the Lua object referenced by a `LUAVALUE`, not the static expectations of the plugin.
- Before calling type-specific access functions such as `luav_asBool()` or `luav_asString()`, plugins should verify the value type using `luav_getType()`.
- Values reported as `LV_OTHERS` are intentionally opaque and cannot be dereferenced or converted using the Lua Value Access Helper API.

## See Also

* [`LUAVALUE`](../data_types)
* [`luav_getType` function](luav_getType)
* [`luav_isNull` function](luav_isNull)
* [Lua Value Access Helper](../lua_helper.md)