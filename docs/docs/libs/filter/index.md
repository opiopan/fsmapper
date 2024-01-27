---
sidebar_position: 1
id: filter_index
---

# filter library
The filter library provides native-actions capable of cascading with other actions.<br/>
The filter library enables processing events, performing conditional branching, and similar operations through cascaded connections of native-actions.
By combining the native-actions provided by the filter library with other native-actions, complex operations can be efficiently executed as native code without the intervention of the Lua interpreter.

## Functions
|Name|Description|
|-|-|
|[```filter.duplicator()```](/libs/filter/filter_duplicator)|Create a native-action that replicates an event for multiple subsequent actions|
|[```filter.branch()```](/libs/filter/filter_branch)|Create a native-action to implement conditional branching between multiple actions|
|[```filter.delay()```](/libs/filter/filter_delay)|Create a native-action that delays the execution of action|
|[```filter.lerp()```](/libs/filter/filter_lerp)|Create a native-action to modify the characteristics curve of a device's analog axis|
