---
id: luacmod_api_index
sidebar_position: 1
---

# Lua C Module

This section documents how fsmapper can be extended using **Lua C modules**.

It is intended for developers who are already familiar with Lua scripting and want to add native functionality to fsmapper using C or C++, especially in cases where asynchronous processing or interaction with external systems is required.

This documentation focuses on the service APIs provided by fsmapper specifically for Lua C modules.

## Background: Lua C Modules and Asynchronous Processing

Lua provides a standard mechanism that allows modules loaded via `require` to be [implemented in native code](https://www.lua.org/manual/5.4/manual.html#6.3).
Such modules are typically built as shared libraries and can expose [Lua C functions](https://www.lua.org/manual/5.4/manual.html#lua_CFunction) and [userdata](https://www.lua.org/manual/5.4/manual.html#2.1) to Lua scripts.    
Using this mechanism, developers can add arbitrary functionality to Lua scripts, independent of fsmapper—for example, device control routines or integrations with external applications.

Lua C modules can freely use all Lua-facing APIs provided by fsmapper, including functions such as [`mapper.raise_event()`](/libs/mapper/mapper_raise_event), as long as execution remains **synchronous** and completes within the call to a Lua function.

However, practical extensions often require **asynchronous processing**:

- Monitoring device input independently of Lua script execution
- Reacting to state changes from external programs
- Performing background I/O or polling operations

In such cases, Lua C modules typically create worker threads.
This introduces a fundamental limitation of the Lua threading model:  
a [`lua_State`](https://www.lua.org/manual/5.4/manual.html#lua_State) may only be accessed safely from the Lua script execution thread, and only while a Lua C function is being executed.

As a result, worker threads cannot directly call Lua APIs such as [`mapper.raise_event()`](/libs/mapper/mapper_raise_event).

## Lua Version and Build Requirements

fsmapper embeds [**Lua 5.4**](https://www.lua.org/manual/5.4/) as its scripting engine.  
All Lua C modules intended to be used with fsmapper **must be built against the [Lua 5.4 C API](https://www.lua.org/manual/5.4/manual.html#4)**.

The fsmapper Plugin SDK provides the required Lua C API header files, such as `lua.h` and `lauxlib.h`, under the following directory:

- `sdk/include`

Developers should use these headers instead of headers from other Lua packages distributed independently of fsmapper.

To use the fsmapper service API for Lua C modules, the following header must also be included:

- `mapperplugin_luac.h` (located in `sdk/include`)

When using C++, Lua headers must be included with C linkage to avoid name mangling.  
A typical include pattern is shown below:

```c title="C++"
extern "C" {
    #include <lua.h>
    #include <lauxlib.h>
}
#include <mapperplugin_luac.h>
```

Both the Lua C API symbols and the fsmapper Lua C module service API are provided by the following library:

- `fsmappercore.lib` (located in `sdk/lib`)

When building with MSVC, the library can be linked explicitly using a pragma directive:

```c title="C/C++"
#pragma comment(lib, "fsmappercore.lib")
```

:::caution Important
fsmapper statically links Lua into its core library.  
Do **not** link your Lua C module against Lua binaries or import libraries provided by other Lua packages.

Lua C modules must be linked against `fsmappercore.lib` and dynamically linked to `fsmappercore.dll` at runtime.
:::

## Service API for Lua C Modules

To address the limitations imposed by the Lua threading model, fsmapper provides a **service API for Lua C modules**.
This API allows Lua C modules to interact with fsmapper **independently of the Lua script execution thread**, enabling safe asynchronous behavior.

The service API is divided into two conceptual parts:

- **Basic Functions**  
  Thread-safe APIs that can be called from any thread, covering:
  - Console logging
  - Emission of events with scalar values

- **Asynchronous Event Source**  
  A mechanism for emitting events whose values are arbitrary Lua objects, while ensuring that all Lua stack interactions occur on the Lua script execution thread.

## Service API Overview

### Basic Functions

Basic Functions provide thread-safe operations that do not require access to the Lua stack.
All functions in this category operate on a module-specific context and may be called from any thread.

|Function|Description|
|--|--|
|[`fsmapper_luac_open_ctx`](./api/fsmapper_luac_open_ctx)|Creates a context required to use the Lua C module service API.|
|[`fsmapper_luac_release_ctx`](./api/fsmapper_luac_release_ctx)|Releases a previously created Lua C service context.|
|[`fsmapper_luac_putLog`](./api/fsmapper_luac_putLog)|Outputs a message to the fsmapper console from any thread.|
|[`fsmapper_luac_send_event`](./api/fsmapper_luac_send_event)|Emits an event without an associated value.|
|[`fsmapper_luac_send_event_int`](./api/fsmapper_luac_send_event_int)|Emits an event with an integer value.|
|[`fsmapper_luac_send_event_float`](./api/fsmapper_luac_send_event_float)|Emits an event with a floating-point value.|
|[`fsmapper_luac_send_event_str`](./api/fsmapper_luac_send_event_str)|Emits an event with a string value.|

For conceptual explanations and usage guidelines, see [Basic Functions](./basic).

### Asynchronous Event Source

The asynchronous event source mechanism is used when an event value must be an **arbitrary Lua object**, such as a table or other composite type.

This mechanism decouples event notification (which may occur on any thread) from Lua stack interaction, which is always performed on the Lua script execution thread.

|Function|Description|
|--|--|
|[`fsmapper_luac_create_async_source`](./api/fsmapper_luac_create_async_source)|Creates an asynchronous event source associated with a Lua C function.|
|[`fsmapper_luac_async_source_signal`](./api/fsmapper_luac_async_source_signal)|Signals that an event is pending from an asynchronous event source.|
|[`fsmapper_luac_release_async_source`](./api/fsmapper_luac_release_async_source)|Releases a previously created asynchronous event source.|

A detailed explanation of this mechanism, including provider function semantics, is available in [Asynchronous Event Source](./async).

## Data Types

Lua C module–specific handle types used by the service API are documented here:

- [`Data Types`](./data_types)

## How to Read This Reference

If you are new to fsmapper’s Lua C module support, the recommended reading order is:

1. [**Basic Functions**](basic)  
   Learn how to create a module context, emit logs, and send scalar-valued events safely from any thread.

2. [**Asynchronous Event Source**](async)  
   Use this mechanism when event values must be Lua objects or when Lua stack interaction is required.

3. **API Reference**  
   Refer to individual function pages for exact semantics, parameters, and constraints.

Together, these sections describe all services provided by fsmapper for extending Lua scripts using native code.