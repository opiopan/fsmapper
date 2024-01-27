---
sidebar_position: 15
---

# mapper.device()
```lua
mapper.device(param_table)
```
This function opens a device and generate a [Device](/libs/mapper/Device) object.


## Prameters
|Parameter|Type|Description|
|-|-|-|
|`param_table`|table|This parameter is in associative array table format, meaning it's specified by keys rather than parameter positions. See the [Parameters Table](#parameters-table) section.|


## Parameters Table
|Key|Type|Description|
|-|-|-|
|`name`|string|User-assigned name for the [Device](/libs/mapper/Device) object.<br/>This parameter is required.
|`type`|string|*Device type**.<br/>This parameter is required.
|`identifier`|table|Specifies the *information for device identification**.<br/>This parameter is required.
|`options`|table|Specifies the *option parameters** for device opening.<br/>This parameter is optional.
|`modifiers`|table|[**Event Modifier**](/guide/device/#event-modifier) array.<br/>It specifies the array table of [Event Modifier Definition](#event-modifier-definition) table.<br/>This parameter is optional

:::note *note
The notation for the values specified in `type`, `identifier`, and `options` varies for each device type.
Refer to the [**Built-in Devices**](/category/built-in-devices) section for details on built-in devices.
For plugin devices, please refer to the plugin's documentation.
:::

### Event Modifier Definition
|Key|Type|Description|
|---|----|-----------|
|`name`|string|Specifies the *name of the [**Device Unit**](/guide/device#device-unit)*** targeted by the modifier.<br/>This parameter and `class` are mutually exclusive.<br/>If an [**Event Modifier**](/guide/device/#event-modifier) is simultaneously defined for the class associated with the [**Device Unit**](/guide/device#device-unit) specified by this parameter, the [**Event Modifier**](/guide/device/#event-modifier) specified by the `name` takes precedence.
|`class`|string|Specifies when applying the same modifier to multiple [**Device Unit**](/guide/device#device-unit)s with similar characteristics.<br/>It specifies one of the following: `binary` for units with binary value ranges, `absolute` for units with absolute value ranges, or `relative` for units with relative value ranges.<br/>This parameter and `name` are mutually exclusive.
|`modtype`|string|Modifier type.<br/>It specifies eather of `raw`, `button`, or `incdec`.
|`modparam`|table|Options specific to the modifier.<br/>For detailed information, refer to the [**Event Modifier Specification**](/guide/device/modifier).

:::note **note
The naming convention of the [**Device Unit**](/guide/device#device-unit) varies for each device type.
Refer to the [**Built-in Devices**](/category/built-in-devices) section for details on built-in devices.
For plugin devices, please refer to the plugin's documentation.
:::

## Return Values
This function returns a [`Device`](/libs/mapper/Device) object corresponding to the specified device.