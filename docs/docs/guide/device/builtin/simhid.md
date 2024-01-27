---
sidebar_position: 6
---

# SimHID Device Specification
The SimHID Devices are input/output devices that support the [**SimHID protocol**](https://github.com/opiopan/simhid-g1000#simhid-protocol).
The [**SimHID protocol**](https://github.com/opiopan/simhid-g1000#simhid-protocol) is adopted by controllers like [**SimHID G1000**](https://github.com/opiopan/simhid-g1000).<br/>
This is the specification for the built-in device driver for SimHID devices.

## Parameters for mapper.device()

### type Parameter
`simhid`

### identifier Parameter
The [`mapper.device()`](/libs/mapper/mapper_device) function takes an associative array type table with the following keys specified as the `identifier` parameter.

|Key|Type|Description|
|---|----|-----------|
|`path`|string|The virtual COM port name where the SimHID device is connected, for instance: `COM2`

### options Parameter
There are currently no options available for specification.

## Device Units
The [**SimHID protocol**](https://github.com/opiopan/simhid-g1000#simhid-protocol) can support various input and output operations, making the provided [**Device Unit**](/guide/device#device-unit)s significantly different for each device.<br/>
Here, I'll focus on explaining the [**Device Unit**](/guide/device#device-unit) offered by the SimHID G1000 as a representative example.

### Buttons
|Attribute|Description|
|--------|-----------|
|Name|The SimHID devices use predefined names directly, meaning they are device-dependent.
|Direction|***Input***
|Value Type|***Absolute***
|Precision|32-bit integer
|Value Range|1 for ON, 0 for OFF

### Rorary Encoders
|Attribute|Description|
|--------|-----------|
|Name|The SimHID devices use predefined names directly, meaning they are device-dependent.
|Direction|***Input***
|Value Type|***Absolute***
|Precision|32-bit integer
|Value Range|When rotating clockwise, the number of clicks is represented by positive integers, when rotating counterclockwise, it's represented by negative integers.
