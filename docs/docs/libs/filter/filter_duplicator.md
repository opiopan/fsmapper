---
sidebar_position: 1
---

# filter.duplicator()
```lua
filter.duplicator(action[, action, ...])
```
This function creates a native-action that replicates an event for multiple subsequent actions.


## Parameters
|Parameter|Type|Description|
|-|-|-|
|`action`|[Action](/guide/event-action-mapping#action)|Specifies the actions to which the replicated events will be delivered. Multiple actions can be specified.

## Return Values
This function returns a native-action.

## See Also
- [Filter](/guide/event-action-mapping#filter)