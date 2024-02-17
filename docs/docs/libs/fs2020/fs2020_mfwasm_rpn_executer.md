---
sidebar_position: 6
---

# fs2020.mfwasm.rpn_executer()
```lua
fs2020.mfwasm.rpn_executer(rpn)
```
This function creates a native-action to execute an [RPN](https://docs.flightsimulator.com/html/Additional_Information/Reverse_Polish_Notation.htm) script within MSFS.

:::warning Note
If you use this function, you need to have [**MobiFlight WASM Module**](https://github.com/MobiFlight/MobiFlight-WASM-Module) installed.
:::

## Prameters
|Parameter|Type|Description|
|-|-|-|
|`rpn`|string|[RPN](https://docs.flightsimulator.com/html/Additional_Information/Reverse_Polish_Notation.htm) script|


## Return Values
This function returns a native-action.

## See Also
- [Interaction with MSFS](/guide/msfs)
- [Changing FS2020's Internal State](/guide/msfs#changing-fs2020s-internal-state)
- [`fs2020.mfwasm.execute_rpn()`](/libs/fs2020/fs2020_mfwasm_execute_rpn)
