---
sidebar_position: 1
id: Device_index
---

# Device object
Device object represents a device such as USB gaming device or custom input/output device.

## Constructors
|Constructor|
|---|
|[`mapper.device()`](/libs/mapper/mapper_device)

## Properties
|Name|Description|
|-|-|
|[```Device.events```](/libs/mapper/Device/Device_events)|A table holding event IDs for each input unit|
|[```Device.upstream_ids```](/libs/mapper/Device/Device_upstream_ids)|A table holding unit IDs for each output unit|

## Methods
|Name|Description|
|-|-|
|[```Device:get_events()```](/libs/mapper/Device/Device-get_events)|Return a table holding event IDs for each input unit|
|[```Device:get_upstream_ids()```](/libs/mapper/Device/Device-get_upstream_ids)|Return a table holding unit IDs for each output unit|
|[```Device:close()```](/libs/mapper/Device/Device-close)|Close the device|
|[```Device:send()```](/libs/mapper/Device/Device-send)|Send a value to output unit|
|[```Device:sender()```](/libs/mapper/Device/Device-sender)|Create a native-action to send a value to output unit|
