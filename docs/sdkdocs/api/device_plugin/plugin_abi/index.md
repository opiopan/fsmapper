---
id: plugin_abi_index
sidebar_position: 1
---

# Plugin ABI

## Plugin Callback Execution Flow {#flow}
The following sequence diagram illustrates when and in what order the plugin callback functions are invoked in response to Lua script execution and device operations.

```mermaid
sequenceDiagram
    participant Lua as Lua Script
    participant FSM as fsmapper
    participant PLG as Device Plugin

    Note over Lua,FSM: Script start
    Note over FSM,PLG: Plugin module loading

    FSM->>PLG: getMapperPluginDeviceOps()
    PLG-->>FSM: MAPPER_PLUGIN_DEVICE_OPS
    FSM->>PLG: FSMDEV_INIT

    Note over Lua,FSM: Device creation

    Lua->>FSM: mapper.device(...)
    FSM->>PLG: FSMDEV_OPEN
    FSM->>PLG: FSMDEV_GET_UNIT_NUM
    PLG-->>FSM: Number of device units
    loop For each device unit
        FSM->>PLG: FSMDEV_GET_UNIT_DEF
        PLG-->>FSM: Device unit definition
    end

    FSM->>PLG: FSMDEV_START
    FSM-->>Lua: Device object

    Note over Lua,FSM: Device operation

    Lua->>FSM: Device::send(unit, value)
    FSM->>PLG: FSMDEV_SEND_UNIT_VALUE

    Note over Lua,FSM: Device destruction

    Lua->>FSM: Device::close() or GC
    FSM->>PLG: FSMDEV_CLOSE

    Note over Lua,FSM: Script termination
    Note over FSM,PLG: Plugin module termination

    FSM->>PLG: FSMDEV_TERM
```