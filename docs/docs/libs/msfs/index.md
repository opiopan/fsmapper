---
sidebar_position: 1
id: msfs_index
---

# msfs library
The msfs library provides interactivity features with Microsoft Flight Simulator.

:::info Note
Previously, these functions were placed under the `fs2020` namespace, not the `msfs` namespace.
Since the features of this library can be used with SimConnect-compatible simulators other than Microsoft Flight Simulator 2020, starting from v1.1.1, 
they have been moved to the `msfs` namespace in preparation for the release of Microsoft Flight Simulator 2024.

For compatibility with the scripts written for older versions, these functions can still be accessed through the `fs2020` namespace, but the documentation will only refer to the `msfs` namespace.
:::

## Functions
|Name|Description|
|-|-|
|[```msfs.send_event()```](/libs/msfs/msfs_send_event)|Send a SimConnect client event|
|[```msfs.event_sender()```](/libs/msfs/msfs_event_sender)|Create a native-action to send a SimConnect client event|
|[```msfs.add_observed_simvars()```](/libs/msfs/msfs_add_observed_simvars)|Register SimVars for observing|
|[```msfs.clear_observed_simvars()```](/libs/msfs/msfs_clear_observed_simvars)|Clear all observed SimVars|
|[```msfs.execute_input_event()```](/libs/msfs/msfs_execute_input_event)|Execute an InputEvent|
|[```msfs.input_event_executer()```](/libs/msfs/msfs_input_event_executer)|Create a native-action to execute an InputEvent|
|[```msfs.mfwasm.execute_rpn()```](/libs/msfs/msfs_mfwasm_execute_rpn)|Execute an RPN script within MSFS|
|[```msfs.mfwasm.rpn_executer()```](/libs/msfs/msfs_mfwasm_rpn_executer)|Create a native-action to execute an RPN script within MSFS|
|[```msfs.mfwasm.add_observed_data()```](/libs/msfs/msfs_mfwasm_add_observed_data)|Register MSFS internal data for observing|
|[```msfs.mfwasm.clear_observed_data()```](/libs/msfs/msfs_mfwasm_clear_observed_data)|Clear all observed MSFS internal data|

## See Also
- [Interaction with MSFS](/guide/msfs)