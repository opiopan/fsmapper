---
sidebar_position: 6
---

# msfs.mfwasm.rpn_executer()
```lua
msfs.mfwasm.rpn_executer(rpn)
```
This function creates a native-action to execute an [RPN](https://docs.flightsimulator.com/html/Additional_Information/Reverse_Polish_Notation.htm) script within MSFS.

:::warning Note
If you use this function, you need to have [**MobiFlight WASM Module**](https://github.com/MobiFlight/MobiFlight-WASM-Module) installed.
:::

## Parameters
|Parameter|Type|Description|
|-|-|-|
|`rpn`|string|[RPN](https://docs.flightsimulator.com/html/Additional_Information/Reverse_Polish_Notation.htm) script|


## Return Values
This function returns a native-action.

## See Also
- [Interaction with MSFS](/guide/msfs)
- [Changing MSFS's Internal State](/guide/msfs#changing-msfss-internal-state)
- [`msfs.mfwasm.execute_rpn()`](/libs/msfs/msfs_mfwasm_execute_rpn)
