---
sidebar_position: 5
---

# DirectInput Game Device Specification
The DirectInput game device is a game device supported by DirectInput.<br/>
This refers to devices listed in the **USB Game Controllers** found in the control panel, i.e. the devices listed in `joy.cpl`.<br/>
This is the specification for the built-in device driver for DirectInput Game Device.

## Parameters for mapper.device()

### type Parameter
`dinput`

### identifier Parameter
The [`mapper.device()`](/libs/mapper/mapper_device) function takes an associative array type table with the following keys specified as the `identifier` parameter.

|Key|Type|Description|
|---|----|-----------|
|`name`|string|The product name of device.<br/>Exclusive with other keys.
|`guid`|string|The GUID of device.<br/>Exclusive with other keys.
|`index`|numeric|The index value within the list of DirectInput game devices managed by Windows.<br/>Exclusive with other keys.<br/>***NOT RECOMMENDED AT ALL***

### options Parameter
The [`mapper.device()`](/libs/mapper/mapper_device) function takes an associative array type table with the following keys specified as the `options` parameter.

|Key|Type|Description|
|---|----|-----------|
|`allowlist`|table|Disable everything except the specified [**Device Unit**](/guide/device#device-unit)s.<br/>Specify an array of Device Unit names like `{‘x’,‘button1’}`.<br/>Mutually exclusive with the `denylist`.
|`denylist`|table|Disable the specified [**Device Unit**](/guide/device#device-unit)s.<br/>Specify an array of Device Unit names like `{‘x’,‘button1’}`.<br/>Mutually exclusive with the `allowlist`.
|`vpovs`|table|Define virtual [**Device Unit**](/guide/device#device-unit)s that emulates a POV, treating two orthogonal analog axes, such as joysticks.<br/> Specify an array of tables in the [**VirtualPov definition table**](#virtualpov-definition-table) format.

#### VirtualPov definition table
|Key|Type|Description|
|---|----|-----------|
|`name`|string|The name assigned to this virtual [**Device Unit**](/guide/device#device-unit)
|`yaxis`|string|The [**Device Unit**](/guide/device#device-unit) name serving as the source for the virtual POV's Y-axis
|`xaxis`|string|The [**Device Unit**](/guide/device#device-unit) name serving as the source for the virtual POV's X-axis
|`resolution`|numeric|The resolution of the POV, indicating the number of directions it can represent.<br/>The default is 4
|`disable_source`|boolean|If set to `true`, it prevents events from the `xaxis` and `yaxis` specified Device Units.<br/> The default is `false`.

## Device Units
:::note
DirectInput game devices consist solely of ***Input*** type [**Device Unit**](/guide/device#device-unit)s.
:::

### Analog Axes
|Attribute|Description|
|--------|-----------|
|Name|DirectInput supports a maximum of eight analog axes, each with the following names:<br/>`'x'`, `'y'`, `'z'`, `'rx'`, `'ry'`, `'rz'`, `'slider1'`, `'slider2'`<br/>Which axes are supported depends on the device.
|Direction|***Input***
|Value Type|***Absolute***
|Precision|32-bit integre
|Value Range|-50,000 to 50,000

### Buttons
|Attribute|Description|
|--------|-----------|
|Name|The naming convention for the button type [**Device Unit**](/guide/device#device-unit) involves appending a number starting from 1 after the word `button`.<br/>For instance, it would be named as `button1` or `button2`.
|Direction|***Input***
|Value Type|***Absolute***
|Precision|32-bit integer
|Value Range|1 for ON, 0 for OFF

### POVs
|Attribute|Description|
|--------|-----------|
|Name|The naming convention for the POV type [**Device Unit**](/guide/device#device-unit) involves appending a number starting from 1 after the word `pov`.<br/>For instance, it would be named as `pov1` or `pov2`.
|Direction|***Input***
|Value Type|***Absolute***
|Precision|32-bit integer
|Value Range|The direction of POV expressed in degrees, meaning values ranging from 0 to less than 360.<br/>If no direction is selected, it is represented as -1.
