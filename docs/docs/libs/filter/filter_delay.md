---
sidebar_position: 3
---

# filter.delay()
```lua
filter.delay(rel_time, action)
```
This function creates a native-action that delays the execution of action.


## Parameters
|Parameter|Type|Description|
|-|-|-|
|`rel_time`|number|Specifies the time in milliseconds to delay the execution of the action.
|`action`|[Action](/guide/event-action-mapping#action)|Specifies the action to be executed with a delayed execution.


## Return Values
This funciton returns a native-action.

## See Also
- [Filter](/guide/event-action-mapping#filter)