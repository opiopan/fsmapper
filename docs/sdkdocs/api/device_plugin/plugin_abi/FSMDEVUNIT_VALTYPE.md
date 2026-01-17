---
id: FSMDEVUNIT_VALTYPE
sidebar_position: 100
---

# FSMDEVUNIT_VALTYPE enumeration

The **`FSMDEVUNIT_VALTYPE`** enumeration defines how a device unit value is interpreted.

This value type determines whether a device unit represents an absolute position, a relative change, or a binary state, and affects how fsmapper processes and applies incoming or outgoing values.

## Syntax
```c
typedef enum {
    FSMDU_TYPE_ABSOLUTE,
    FSMDU_TYPE_RELATIVE,
    FSMDU_TYPE_BINARY,
} FSMDEVUNIT_VALTYPE;
````

## Enumerators

| Name                | Description                                                                                                                                                       |
| ------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| FSMDU_TYPE_ABSOLUTE | Represents an absolute value within a defined range. Typical examples include joystick axes or sliders that continuously report their current position.           |
| FSMDU_TYPE_RELATIVE | Represents a relative change (delta) rather than an absolute position. Typical examples include mouse movement or rotary encoders that report incremental motion. |
| FSMDU_TYPE_BINARY   | Represents a binary state with values of 0 or 1. Typical examples include buttons, switches, or on/off signals.                                                   |

## Remarks

- For absolute units, the value represents the current state of the unit and typically remains meaningful even when no recent changes have occurred.

- For relative units, each reported value represents a change amount and is interpreted cumulatively by fsmapper.

- Binary units are treated as a special case of absolute values with a fixed range of 0 or 1.

- The value type is used by fsmapper to determine how values are propagated, accumulated, and exposed to Lua scripts.

## See Also

* [`FSMDEVUNITDEF` structure](FSMDEVUNITDEF)
* [`FSMDEVUNIT_DIRECTION` enum](FSMDEVUNIT_DIRECTION)
* [`FSMDEV_GET_UNIT_DEF` callback function](FSMDEV_GET_UNIT_DEF)

