---
id: MAPPER_PLUGIN_DEVICE_OPS
sidebar_position: 500
---

# MAPPER_PLUGIN_DEVICE_OPS structure

## Syntax
```c
typedef struct {
    const char* name;
    const char* description;
    FSMDEV_INIT init;
    FSMDEV_TERM term;
    FSMDEV_OPEN open;
    FSMDEV_START start;
    FSMDEV_CLOSE close;
    FSMDEV_GET_UNIT_NUM getUnitNum;
    FSMDEV_GET_UNIT_DEF getUnitDef;
    FSMDEV_SEND_UNIT_VALUE sendUnitValue;
} MAPPER_PLUGIN_DEVICE_OPS;
```

## Members

## Remarks

## See also
- [`FSMDEV_INIT` callback function](FSMDEV_INIT)
- [`FSMDEV_TERM` callback function](FSMDEV_TERM)
- [`FSMDEV_OPEN` callback function](FSMDEV_OPEN)
- [`FSMDEV_START` callback function](FSMDEV_START)
- [`FSMDEV_CLOSE` callback function](FSMDEV_CLOSE)
- [`FSMDEV_GET_UNIT_NUM` callback function](FSMDEV_GET_UNIT_NUM)
- [`FSMDEV_GET_UNIT_DEF` callback function](FSMDEV_GET_UNIT_DEF)
- [`FSMDEV_SEND_UNIT_VALUE` callback function](FSMDEV_SEND_UNIT_VALUE)
- [`getMapperPluginDeviceOps` function](getMapperPluginDeviceOps)
