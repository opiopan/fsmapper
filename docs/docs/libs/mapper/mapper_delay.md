---
sidebar_position: 6
---

# mapper.delay()
```lua
mapper.delay(rel_time, func)
```
This function calls the function specified by `func` after the time specified by `rel_time``.
The function itself returns immediately.

:::warning Note
fsmapper and Windows are not designed to guarantee real-time constraints.
Thus, while the function specified by `func` won’t execute until the specified time `rel_time`, the actual execution time depends on system load.
:::

## Prameters
|Parameter|Type|Description|
|-|-|-|
|`rel_time`|number|Specifies the time to delay the execution of the function in milliseconds.|
|`func`|function|Specifies the function targeted for delayed execution.|


## Return Values
This function doesn't return any value.