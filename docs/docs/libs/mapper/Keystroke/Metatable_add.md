---
sidebar_position: 5
---

# + operator

```lua
lval + rval
```

fsmapper overrides the `__add` function of the metatable for [`Keystroke`](/libs/mapper/Keystroke) objects, allowing the use of the `+` operator on [`Keystroke`](/libs/mapper/Keystroke) objects.
You can combine two [`Keystroke`](/libs/mapper/Keystroke) objects to create a new [`Keystroke`](/libs/mapper/Keystroke) object by using the `+` operator.

## Parameters
|Parameter|Type|Description|
|-|-|-|
|`lval`|[`Keystroke`](/libs/mapper/Keystroke)|Left operand
|`rval`|[`Keystroke`](/libs/mapper/Keystroke)|Right operand


## Return Values
This operator returns a [`Keystroke`](/libs/mapper/Keystroke) object.