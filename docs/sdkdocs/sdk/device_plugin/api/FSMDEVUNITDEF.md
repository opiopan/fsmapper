---
id: FSMDEVUNITDEF
sidebar_position: 100
---

# FSMDEVUNITDEF structure

The **`FSMDEVUNITDEF`** structure describes the static characteristics of a device unit exposed by a custom device plugin.

In addition to defining the unit’s direction, value type, and valid range, this structure assigns a unique name to each device unit.  
The unit name plays a central role in the fsmapper–Lua interface, as Lua scripts use it as a key to access device unit information, such as event ID for input units and unit ID for output units.


## Syntax
```c
typedef struct {
    const char* name;
    FSMDEVUNIT_DIRECTION direction;
    FSMDEVUNIT_VALTYPE type;
    int maxValue;
    int minValue;
} FSMDEVUNITDEF;
````

## Members

| Member|Type|Description|
|--|--|--|
| name | const char* | A null-terminated string that uniquely identifies the device unit within the device. This name is used by fsmapper and Lua scripts to refer to the unit.
| direction | [`FSMDEVUNIT_DIRECTION`](FSMDEVUNIT_DIRECTION) | Specifies the data flow direction of the unit from the fsmapper and Lua perspective.
| type | [`FSMDEVUNIT_VALTYPE`](FSMDEVUNIT_VALTYPE)     | Specifies how the unit value is interpreted, such as absolute, relative, or binary.
| maxValue | int | The maximum valid value for the unit. The interpretation depends on the unit value type.
| minValue | int | The minimum valid value for the unit. The interpretation depends on the unit value type.

## Remarks

* This structure is filled by the plugin implementation in the [`FSMDEV_GET_UNIT_DEF`](FSMDEV_GET_UNIT_DEF) callback.
* The memory pointed to by `name` must remain valid for the lifetime of the device.
* For absolute and binary units, `minValue` and `maxValue` define the valid value range.
* For relative units, `minValue` and `maxValue` indicate the expected range of delta values.
* fsmapper uses this definition to determine how device unit values are processed and exposed to Lua scripts.

## See Also

* [`FSMDEV_GET_UNIT_DEF` callback function](FSMDEV_GET_UNIT_DEF)
* [`FSMDEVUNIT_DIRECTION` enum](FSMDEVUNIT_DIRECTION)
* [`FSMDEVUNIT_VALTYPE` enum](FSMDEVUNIT_VALTYPE)
* [Device Unit](/guide/device/#device-unit)

