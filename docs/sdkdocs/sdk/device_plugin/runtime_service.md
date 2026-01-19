---
id: runtime_service
sidebar_position: 200
---

# fsmapper Runtime Service

The fsmapper runtime service provides a set of utility functions that plugin modules can call at runtime.
These functions allow plugins to interact with the fsmapper core in a controlled and ABI-stable manner,
without depending on internal data structures or execution context.

The runtime service API is intentionally small and clearly scoped.
Its primary role is to support context management, logging, script control, and device state notification.

All runtime service functions are **thread-safe** and may be safely called from any thread,
including threads other than the one invoking plugin callback functions.

## Context Management

The runtime service provides APIs for associating plugin-defined context data with fsmapper-managed objects.
Two levels of context are supported:

- **Module-level context**, shared across the entire plugin module
- **Device-level context**, associated with an individual device instance

These functions are typically used to bind plugin-specific state to the fsmapper lifecycle
and to retrieve that state later during callback execution.

| Function | Description |
|--|--|
| [`fsmapper_setContext`](./api/fsmapper_setContext) | Associates a plugin-defined context pointer with the plugin module instance. |
| [`fsmapper_getContext`](./api/fsmapper_getContext) | Retrieves the context pointer previously associated with the plugin module instance. |
| [`fsmapper_setContextForDevice`](./api/fsmapper_setContextForDevice) | Associates a plugin-defined context pointer with a specific device instance. |
| [`fsmapper_getContextForDevice`](./api/fsmapper_getContextForDevice) | Retrieves the context pointer previously associated with a device instance. |

## Device Event Notification

For INPUT-type device units, plugins must explicitly notify fsmapper when the device state changes.
These notifications are propagated to the Lua runtime as input events.

| Function | Description |
|--|--|
| [`fsmapper_raiseEvent`](./api/fsmapper_raiseEvent) | Notifies fsmapper of a state change in an INPUT device unit. |

See the detailed reference for lifetime rules and ownership considerations.

## Console Logging

The runtime service provides a unified logging facility for plugin modules.
Log messages emitted through this API are routed to the fsmapper console and log output,
ensuring consistent formatting and visibility.

| Function | Description |
|--|--|
| [`fsmapper_putLog`](./api/fsmapper_putLog) | Writes a log message to the fsmapper console with the specified log level. |

## Script Termination Control

In certain error conditions, a plugin may determine that continued script execution is unsafe or meaningless.
The runtime service provides a mechanism to immediately abort the currently running Lua script.

| Function | Description |
|--|--|
| [`fsmapper_abort`](./api/fsmapper_abort) | Requests immediate termination of the currently executing Lua script. |

This function is intended for unrecoverable errors and should be used sparingly.

## Thread Safety

All runtime service functions are fully thread-safe.
A plugin may invoke these functions from any thread context,
regardless of which thread the original callback was executed on.

This design allows plugin implementations to perform asynchronous I/O,
background processing, or hardware polling without imposing additional synchronization constraints.
