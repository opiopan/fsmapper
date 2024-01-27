---
sidebar_position: 2
---

# filter.branch()
```lua
filter.branch(cond-exp[, cond-exp, ...])
```
This function creates a native-action to implement conditional branching between multiple actions.<br/>

This filter can handle events with numerical values.
You can specify one or more conditions based on the increase or decrease of the event value beyond or below a certain threshold, and trigger the specified actions accordingly.

## Prameters
|Parameter|Type|Description|
|-|-|-|
|`cond-exp`|table|Specifies an associative array of the [Condition Expression](#condition-expression) type representing the action and the conditions under which it is invoked.

### Condition Expression
|Parameter|Type|Description|
|-|-|-|
|`condition`|string|Specifies the condition under which the `action` is called when the Event Value exceeds the threshold specified by `value` from which direction.<br/>`'exeeded'` represents the condition where the threshold is exceeded by an increase in the Event Value, and `'falled'` represents the condition where the threshold is exceeded by a decrease in the Event Value.<br/>This parameter is required.
|`value`|numeric|Threshold for the condition that triggers the action.
|`action`|[Action](/guide/event-action-mapping#action)|The action to be called when the condition is met.

## Return Values
This function returns a native-action.