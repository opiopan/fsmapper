---
id: FSMDEVUNIT_DIRECTION
sidebar_position: 100
---

# FSMDEVUNIT_DIRECTION enumeration

The **`FSMDEVUNIT_DIRECTION`** enumeration defines the direction of data flow of a device unit **from the perspective of fsmapper and Lua scripts**.

This direction determines whether a device unit represents values flowing *into* fsmapper from the plugin, or values sent *out* from fsmapper to the plugin via callback invocation.

## Syntax
```c
typedef enum {
    FSMDU_DIR_INPUT,
    FSMDU_DIR_OUTPUT,
} FSMDEVUNIT_DIRECTION;
````

## Enumerators

|Name|Description|
|--|--|
| FSMDU_DIR_INPUT  | Indicates an input unit that delivers values **from the plugin to fsmapper**. The plugin asynchronously notifies value changes by [`fsmapper_raiseEvent`](../runtime_service/fsmapper_raiseEvent) function. 
| FSMDU_DIR_OUTPUT | Indicates an output unit that receives values **from fsmapper to the plugin**. Value changes are delivered through the [`FSMDEV_SEND_UNIT_VALUE`](FSMDEV_SEND_UNIT_VALUE) callback when Lua code updates the unit state.

## Remarks

- The direction is defined from the fsmapper/Lua point of view, not from the physical device or plugin implementation perspective.

- For input units, the plugin acts as the value producer and reports state changes to fsmapper asynchronously.

- For output units, fsmapper invokes plugin callbacks to notify value changes requested by Lua scripts.

- This direction information is used by fsmapper to determine how device units participate in the execution flow and how state changes are propagated.

## See Also

* [`FSMDEVUNITDEF` structure](FSMDEVUNITDEF)
* [`FSMDEV_GET_UNIT_DEF` callback function](FSMDEV_GET_UNIT_DEF)
* [`FSMDEV_SEND_UNIT_VALUE` callback function](FSMDEV_SEND_UNIT_VALUE)
* [`fsmapper_raiseEvent` function](../runtime_service/fsmapper_raiseEvent)
