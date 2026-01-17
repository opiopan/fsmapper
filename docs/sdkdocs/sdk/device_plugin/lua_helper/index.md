---
id: lua_helper_index
sidebar_position: 1
---

# Lua Value Access Helper

The **Lua Value Access Helper** provides a small set of utility functions for *reading* values from Lua objects referenced by [**`LUAVALUE`**](../data_types).

This helper is primarily intended for interpreting the **`identifier`** and **`options`** parameters passed to the [**`FSMDEV_OPEN`**](../plugin_abi/FSMDEV_OPEN) callback function.
These parameters are supplied as Lua objects, and [`LUAVALUE`](../data_types) represents references to those objects from C/C++ code.

The helper is intentionally simple and data-oriented.  
It allows C/C++ code to inspect and read Lua values, but does **not** provide facilities for updating values, invoking functions, or performing arbitrary Lua operations.

:::warning Constraints

- The Lua Value Access Helper functions are **not thread-safe**.
  They must not be called concurrently from multiple threads.

- All functions in this category are valid **only during the execution of the [`FSMDEV_OPEN`](../plugin_abi/FSMDEV_OPEN) callback**.  
  Once the [`FSMDEV_OPEN`](../plugin_abi/FSMDEV_OPEN) callback function returns, any [`LUAVALUE`](../data_types) obtained from its parameters becomes invalid and **must not be accessed**.

Violating these constraints may result in undefined behavior.

:::

## Value Type Inspection

| Function | Description |
|--------|-------------|
| [`luav_getType`](./luav_getType) | Returns the type of the Lua value referenced by a `LUAVALUE`. |
| [`luav_isNull`](./luav_isNull) | Checks whether the referenced Lua value is `nil`. |

## Scalar Value Access

| Function | Description |
|--------|-------------|
| [`luav_asBool`](./luav_asBool) | Reads the Lua value as a boolean. |
| [`luav_asInt`](./luav_asInt) | Reads the Lua value as a 64-bit integer. |
| [`luav_asDouble`](./luav_asDouble) | Reads the Lua value as a double-precision floating-point number. |
| [`luav_asString`](./luav_asString) | Reads the Lua value as a null-terminated string. |

## Table Access

| Function | Description |
|--------|-------------|
| [`luav_getItemWithKey`](./luav_getItemWithKey) | Retrieves a table element by string key and returns it as a `LUAVALUE`. |
| [`luav_getItemWithIndex`](./luav_getItemWithIndex) | Retrieves a table element by numeric index and returns it as a `LUAVALUE`. |

---
The Lua Value Access Helper is designed as a lightweight, read-only bridge between Lua data and C/C++ code, with a specific focus on configuration and identification data supplied to device open callbacks.