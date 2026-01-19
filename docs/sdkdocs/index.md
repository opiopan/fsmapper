---
id: sdk-overview
sidebar_position: 1
---

# Plugin SDK Overview

The fsmapper Plugin SDK provides native extension mechanisms that allow developers to extend fsmapper capability.

Using the Plugin SDK, developers can integrate custom native code into fsmapper in a structured and supported way, while leveraging fsmapper’s device model, event-driven architecture, and Lua scripting environment.

This page provides a high-level overview of the Plugin SDK and serves as an entry point to the tutorial and reference documentation.



## What the Plugin SDK Provides

The Plugin SDK enables developers to implement **plugin modules** that extend fsmapper using native code written in C or C++.

Through the SDK, plugin modules can:

- Integrate with fsmapper’s internal device and event model
- Emit events and interact with Lua scripts
- Implement functionality that requires native performance or asynchronous processing

The SDK does not replace Lua scripting; instead, it complements it by providing extension points where native code is more suitable.



## Supported Plugin Types

The fsmapper Plugin SDK supports two distinct types of plugin modules.

### Custom Device Plugin

Custom Device Plugins allow developers to define [**new device types**](/guide/device/#device-type) that integrate directly into fsmapper’s device model.

These plugins are typically used to represent physical devices, virtual devices, or external systems as fsmapper devices.
They interact with fsmapper through a well-defined plugin ABI and are invoked in response to Lua script operations.

Detailed documentation is available under:

- [**Custom Device Plugin**](./sdk/device_plugin/) (SDK Reference)



### Lua C Module

Lua C Modules extend Lua scripts executed by fsmapper using native code.

They are loaded through Lua’s standard module mechanism and can provide Lua functions, userdata, and asynchronous event sources.
This approach is particularly useful for implementing asynchronous processing, background tasks, or integrations with external systems.

Detailed documentation is available under:

- [**Lua C Module**](./sdk/lua_cmod/) (SDK Reference)



## Choosing a Plugin Type

The fsmapper Plugin SDK provides two different extension mechanisms.
Both allow native code to integrate with fsmapper and both can perform asynchronous processing and emit events.

The primary difference between them is **how tightly they integrate with fsmapper’s device model**, which directly affects **flexibility and expressive power**.

This section provides guidelines to help you choose the plugin type that best fits your design goals.



### Custom Device Plugin

A [**Custom Device Plugin**](./sdk/device_plugin/) is designed to integrate native functionality into fsmapper’s [**device model**](/guide/device/).

By following this model, a plugin can take advantage of fsmapper’s built-in mechanisms, such as:

- Flexible event mapping through [**event modifiers**](/guide/device/#event-modifier)
- A consistent representation of inputs and outputs as device units
- Seamless integration with [`mapper.device()`](/libs/mapper/mapper_device) in Lua scripts

This structured integration is powerful when your functionality naturally fits the concept of a device.

However, this structure also imposes constraints.
Communication between fsmapper and a device plugin is based on [**device units**](/guide/device/#device-unit), and the values exchanged for each unit are limited to integer values.
This makes Custom Device Plugins less suitable for cases where:

- Events need to carry complex or structured data
- Rich bidirectional commands must be sent from fsmapper to the plugin
- The interaction model does not align well with the notion of device units

In short, Custom Device Plugins trade **expressive freedom** for **deep integration and consistency** within fsmapper’s device ecosystem.



### Lua C Module

A [**Lua C Module**](./sdk/lua_cmod/) integrates at the **Lua scripting level**, without being constrained by fsmapper’s device abstraction.

This approach offers significantly more flexibility:

- Events can carry arbitrary Lua objects, including tables and composite data
- Interaction with Lua scripts can follow any structure or protocol
- The module can expose custom Lua functions and userdata
- Asynchronous processing can be implemented without conforming to the device unit model

Because Lua C Modules operate closer to Lua itself, they are often a natural choice when:

- The problem does not fit well into fsmapper’s device model
- Complex data structures must be exchanged via events
- Fine-grained or custom control logic is required

Developers who are already familiar with Lua C modules or the Lua C API may also find this approach easier, as it relies more on standard Lua extension techniques and less on fsmapper-specific plugin interfaces.



### Summary

At a high level:

- Choose a [**Custom Device Plugin**](./sdk/device_plugin/) when you want to extend fsmapper’s **device model** and benefit from its built-in mapping, filtering, and modifier mechanisms.
- Choose a [**Lua C Module**](./sdk/lua_cmod/) when you need **maximum flexibility**, complex data exchange, or tight integration with Lua scripts.

Both plugin types can coexist and complement each other.
Selecting the appropriate one helps keep plugin designs aligned with fsmapper’s architecture and your implementation goals.


## Obtaining and Using the Plugin SDK

The fsmapper Plugin SDK is **installed together with fsmapper**.
No separate download or installation step is required.

After installing fsmapper, the Plugin SDK can be found under the fsmapper installation directory as the `sdk` folder.

```
fsmapper
└── sdk
    ├── include
    ├── lib
    └── samples
```

- **include**  
  Contains header files required to develop plugin modules.

- **lib**  
  Contains libraries needed to link plugin modules against fsmapper.

- **samples**  
  Contains sample projects demonstrating how to implement supported plugin types.

Developers are free to copy the entire `sdk` folder to any location and use it independently of the fsmapper installation directory.
This can be convenient when building samples or plugin modules outside the fsmapper installation folder.



## How to Read This Documentation

This documentation is organized into three layers:

1. **Overview**  
   High-level explanations of concepts and architecture (**this page**).

2. [**Tutorials**](./tutorial/)  
   Step-by-step guides based on the sample code included with the Plugin SDK.

3. [**SDK Reference**](./sdk/)  
   Detailed reference documentation for APIs, data types, and conventions.

If you are new to the Plugin SDK, it is recommended to start with the tutorials before referring to the SDK reference.



## Next Steps

To continue, proceed to one of the following sections depending on your goal:

- Learn by example using the included sample code: [**Tutorials**](./tutorial/)
- Look up detailed API specifications: [**Plugin SDK Reference**](./sdk/)

These sections provide the practical and detailed information needed to implement plugin modules using the fsmapper Plugin SDK.