---
id: FSMAPPER_LOG_TYPE
sidebar_position: 100
---

# FSMAPPER_LOG_TYPE enumeration

The **`FSMAPPER_LOG_TYPE`** enumeration defines the severity levels used when emitting log messages from a plugin module to the fsmapper console.

These values allow plugins to categorize log output by importance and intent, helping developers and users distinguish errors, warnings, informational messages, and debug output.

## Syntax
```c
typedef enum {
    FSMLOG_ERROR,
    FSMLOG_WARNING,
    FSMLOG_INFO,
    FSMLOG_MESSAGE,
    FSMLOG_DEBUG,
} FSMAPPER_LOG_TYPE;
```

## Enumerators

|Enumerator|Description|
|--|--|
|`FSMLOG_ERROR`|Indicates a fatal or critical error condition that prevents correct operation of the plugin or device.|
|`FSMLOG_WARNING`|Indicates a non-fatal issue or unexpected condition that may affect behavior but does not immediately stop execution.|
|`FSMLOG_INFO`|Provides informational messages about normal plugin or device operation.|
|`FSMLOG_MESSAGE`|Represents general user-facing messages that are neither warnings nor errors.|
|`FSMLOG_DEBUG`|Provides detailed diagnostic output intended for plugin development and debugging.|

## Remarks

- The interpretation and presentation of each log level are handled by fsmapper.
- Plugins should select the most appropriate log level to clearly communicate the significance of each message.
- Excessive use of debug-level logging is discouraged in production environments.

## See Also

- [`fsmapper_putLog` function](fsmapper_putLog)
