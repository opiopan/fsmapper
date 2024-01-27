---
sidebar_position: 24
---

# mapper.keystroke()
```lua
mapper.keystroke(param_table)
```
This function creates a [`Keystroke`](/libs/mapper/Keystroke) object for keybord emulation.<br/>
Keystroke object represents

## Prameters
|Parameter|Type|Description|
|-|-|-|
|`param_table`|table|This parameter is in associative array table format, meaning it's specified by keys rather than parameter positions. See the [Parameters Table](#parameters-table) section.|


## Parameters Table
|Key|Type|Description|
|-|-|-|
|codes|table|Specifies an array table of [virtual-key names](/guide/input_emulation/keycodes) representing the sequence of key inputs to emulate.<br/>This parameter is required.
|modifiers|table|Specifies the keys to be modified when generating the key input sequence specified by the `codes` parameter. This is done using an array table of [virtual-key names](/guide/input_emulation/keycodes) representing the modifier keys*.
|duration|numeric|Specifies the time in milliseconds to hold the key down before releasing it.<br/>The default is `50`.
|interval|numeric|Specifies the time in milliseconds to wait after releasing one key before pressing the next key in the sequence.<br/>The default is `0`.

:::note *
The virtual-keys that can be specified as modifier keys are as follows.
- `'VK_SHIFT'`
- `'VK_LSHIFT'`
- `'VK_RSHIFT'`
- `'VK_CONTROL'`
- `'VK_LCONTROL'`
- `'VK_RCONTROL'`
- `'VK_MENU'`
- `'VK_LMENU'`
- `'VK_RMENU'`
- `'VK_LWIN'`
- `'VK_RWIN'`
:::

:::tip
When specifying virtual-key names, you can omit the `VK_` prefix. The following two Keystroke objects generate the same key input sequence.

```lua
local keys1 = mapper.Keystroke{codes = {'VK_UP', 'VK_RIGHT'}}
local keys2 = mapper.Keystroke{codes = {'UP', 'RIGHT'}}
```
:::

## Return Values
This function returns [`Keystroke`](/libs/mapper/Keystroke) object.