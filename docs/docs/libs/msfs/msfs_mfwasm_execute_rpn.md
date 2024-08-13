---
sidebar_position: 5
---

# msfs.mfwasm.execute_rpn()
```lua
msfs.mfwasm.execute_rpn(rpn)
```
This function executes an [RPN](https://docs.flightsimulator.com/html/Additional_Information/Reverse_Polish_Notation.htm) script within MSFS.

:::warning Note
If you use this function, you need to have [**MobiFlight WASM Module**](https://github.com/MobiFlight/MobiFlight-WASM-Module) installed.
:::

## Parameters
|Parameter|Type|Description|
|-|-|-|
|`rpn`|string|[RPN](https://docs.flightsimulator.com/html/Additional_Information/Reverse_Polish_Notation.htm) script|


## Return Values
This function doesn't return any value.

## See Also
- [Interaction with MSFS](/guide/msfs)
- [Changing MSFS's Internal State](/guide/msfs#changing-msfss-internal-state)
- [`msfs.mfwasm.rpn_executer()`](/libs/msfs/msfs_mfwasm_rpn_executer)
