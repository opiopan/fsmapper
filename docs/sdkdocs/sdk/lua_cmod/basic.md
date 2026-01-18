---
id: basic
sidebar_position: 100
---

# Basic Functions

This section describes the **Basic Functions** provided by fsmapper for Lua C modules.

Basic Functions are designed to support asynchronous extensions implemented in native code while avoiding direct interaction with the Lua stack.
All functions in this category are **thread-safe** and may be called from any thread, including worker threads created by a Lua C module.


## Overview

When extending fsmapper using a Lua C module, synchronous operations can usually be completed directly within a Lua C function call.
However, once asynchronous processing is involved—such as background device monitoring or external event handling—native code often needs to operate outside the Lua script execution thread.

Basic Functions address this requirement by providing APIs that:

- Do not require access to [`lua_State`](https://www.lua.org/manual/5.4/manual.html#lua_State)
- Are safe to call from arbitrary threads
- Allow Lua C modules to interact with fsmapper in a controlled and well-defined manner

These functions form the foundation of fsmapper’s Lua C module service API.


## Service Context

All Basic Functions operate on a **service context**, represented by a
[`FSMAPPER_LUAC_CTX`](./data_types) handle.

A service context represents an execution context required to use the fsmapper service APIs for Lua C modules.
It does **not** represent the lifetime of a Lua module itself, nor is it restricted to a single module-wide instance.

A service context is created explicitly using
[`fsmapper_luac_open_ctx`](./api/fsmapper_luac_open_ctx).
Once created, the context can be freely associated with any Lua-side object, such as a userdata, and multiple service contexts may coexist at the same time.
Creating multiple contexts—for example, one per userdata—is fully supported and within the intended usage model.

Internally, fsmapper uses the service context to associate service API calls with the originating Lua C module, primarily for purposes such as logging and event emission.

The lifetime of a service context is managed explicitly by the module:

- A service context may be created at any time, independent of module initialization
- The same context is passed to all subsequent service API calls that require it
- Each created context **must** be released using
  [`fsmapper_luac_release_ctx`](./api/fsmapper_luac_release_ctx)

Lua does not provide a direct mechanism to notify a module when a script terminates.
As a result, it is the responsibility of the Lua C module to ensure that all service contexts are released no later than script termination.

A common and recommended pattern is to associate a service context with a userdata object and release it from the userdata’s [`__gc` metamethod](https://www.lua.org/manual/5.4/manual.html#2.5.3).
This ensures that any resources allocated on the fsmapper side are properly reclaimed when the Lua state is closed.

The service context handle itself is opaque and does not expose any internal state to the module.


## Thread-Safe Logging

Basic Functions include an API for outputting log messages to the fsmapper console from any thread.

The following function can be used to emit log messages from worker threads or asynchronous callbacks:

|Function|Description|
|--|--|
|[`fsmapper_luac_putLog`](./api/fsmapper_luac_putLog)|Outputs a log message to the fsmapper console|


## Emitting Events with Scalar Values

Basic Functions also provide APIs for emitting fsmapper events whose values are **scalar types**.
These functions do not involve the Lua stack and can therefore be called safely from any thread.

Internally, fsmapper copies the provided event data as needed.
As a result, any memory passed to these functions may be safely released once the function call returns.

The following functions are available for emitting scalar-valued events:

|Function|Description|
|--|--|
|[`fsmapper_luac_send_event`](./api/fsmapper_luac_send_event)|Emits an event without an associated value.|
|[`fsmapper_luac_send_event_int`](./api/fsmapper_luac_send_event_int)|Emits an event with an integer value.|
|[`fsmapper_luac_send_event_float`](./api/fsmapper_luac_send_event_float)|Emits an event with a floating-point value.|
|[`fsmapper_luac_send_event_str`](./api/fsmapper_luac_send_event_str)|Emits an event with a string value.|

These APIs are suitable when the event value does not need to be represented as a Lua object.


## When to Use Basic Functions

Basic Functions are appropriate in the following situations:

- The event value can be represented as a scalar value (or no value)
- Event emission originates from a worker thread
- No interaction with the Lua stack is required
- Simplicity and thread safety are prioritized

If an event must carry an **arbitrary Lua object** as its value, Basic Functions are not sufficient.
In such cases, the [**Asynchronous Event Source**](async) mechanism should be used instead.