---
id: device_index
sidebar_position: 3
---

# Device Handling
In this section, the explanation will cover how fsmapper deals with flight sticks and custom control devices.

fsmapper provides a [`Device`](/libs/mapper/Device/) object to abstract various devices, including those supported by the OS and custom-built devices, for unified handling.
Users generate a [`Device`](/libs/mapper/Device/) corresponding to the desired device using [`mapper.device()`](/libs/mapper/mapper_device) for each device they want to use.

Below is an example of opening a Direct Input game controller device, which was also mentioned in the [**Tutorial**](/getting-started/tutorial#handle-an-input-device).

```lua
device = mapper.device{
    name = 'Tutorial',
    type = 'dinput',
    identifier = {index = 1},
    modifiers = {},
}
```

The argument for [`mapper.device()`](/libs/mapper/mapper_device) is a table in associative array format, where each key-value pair of the associative array represents an actual argument. Here's a brief overview of each key in the table.

|Key|Description|Requirement|
|---|-----------|:-:|
|`name`|User-assigned name for the Device object|**REQUIRED**
|`type`|Type of the device|**REQUIRED**
|`identifier`|Information for device identification|**REQUIRED**
|`options`|Option parameters for device opening|optional
|`modifiers`|Event modifier|optional

Further, we'll delve into the specifics of the values to be specified in this argument and other essential aspects to know for manipulating devices.

## Device Type
fsmapper uses pluggable driver modules to handle device-specific operations and absorb differences between devices. The `type` parameter for [`mapper.device()`](/libs/mapper/mapper_device) specifies the device type, i.e., which driver to use.

fsmapper comes with the following built-in drivers.

|Type Name|Description|
|---------|-----------|
|`dinput`|Game devices supported by DirectInput.<br/>This refers to devices listed in the **USB Game Controllers** found in the control panel, i.e. the devices listed in `joy.cpl`.
|`simhid`|Input/output devices that support the [**SimHID protocol**](https://github.com/opiopan/simhid-g1000#simhid-protocol).<br/> The [**SimHID protocol**](https://github.com/opiopan/simhid-g1000#simhid-protocol) is adopted by controllers like [**SimHID G1000**](https://github.com/opiopan/simhid-g1000) that I've DIY'd.


The interpretation of the `identifier` and `options` parameters is performed within each driver. 
The meaning of the values varies depending on the device type.<br/>
The `identifier` parameter specifies information necessary for the driver to identify the actual device. 
For example, for a `dinput` device, it might specify the product name or GUID, while for a `simhid` device, it might specify the virtual COM port name.<br/>
The 'options' parameter is used to convey driver-specific optional information.

For detailed specifications on what to specify in the `identifier` and `options` parameters for built-in drivers, please refer to the [**DirectInput Game Device Specification**](builtin/dinput) and the [**SimHID Device Specification**](builtin/simhid).
For devices provided by plugin modules, refer to the description of those respective modules.

## Device Unit
**Device Unit** refers to the constituent elements of devices with changing states, such as buttons or analog axes on a joystick.
Each **Device Unit** owns attributes including **Name**, **Direction**, **Value Type**, **Precision**, **Value Range**.

- **Name**<br/>
    **Name** corresponds to the ID of each **Device Unit**, and the naming convention depends on the device type.

- **Direction**<br/>
    **Direction** indicates whether it is ***Input*** or ***Output***, signifying whether the state is obtained from the PC or if the state can be altered by the PC.<br/>
    **Device Unit**s of the ***Input*** type notify changes in state to the fsmapper system as [**Event**s](/guide/event-action-mapping#event).

- **Value Type**<br/>
    **Value Type** can be ***Absolute*** or ***Relative***. ***Absolute*** refers to instances where the value of the **Device Unit** directly corresponds to its position, such as analog axes on joysticks. ***Relative*** describes cases like a mouse, where the change in value represents the **Device Unit**'s value.

- **Precision**<br/>
    **Precision** signifies the precision of the **Device Unit**'s state value, represented by either a 32-bit integer or a 64-bit floating-point number.

- **Values Range**<br/>
    **Value Range** indicate the range within which the **Device Unit** can express values.

The attributes of **Device Unit**s corresponding to constituent elements of actual devices vary depending on the device type.<br/>
For devices supported by built-in drivers, refer to the [**DirectInput Game Device Specification**](builtin/dinput) and the [**SimHID Device Specification**](builtin/simhid).
For devices provided by plugin modules, refer to the description of those respective modules.

:::tip
For ***Input*** type **Device Unit**s, observing the events generated while manipulating the unit with `Show Events and Messages` enabled in the **Message Console** can aid in understanding the specifications.
:::

## Event Modifier
**Event Modifier** is a mechanism that determines how the state changes of a [**Device Unit**](#device-unit) are translated into specific events. 
There are three types of **Event Modifiers**: `raw`, `button`, and `incdec`. 
You can specify which modifier to use in the `modifiers` parameter of [`mapper.device()`](/libs/mapper/mapper_device).

- **`raw`**<br/>
    This is the default modifier.
    It triggers a `change` event every time the value of the [**Device Unit**](#device-unit) changes.
    The [**Event Value**](/guide/event-action-mapping#event) is directly set as the value of the [**Device Unit**](#device-unit).

- **`button`**<br/>
    This modifier is intended for application to [**Device Unit**](#device-unit)s that can be represented by binary values, such as buttons. 
    Its basic functionality is to generate a `down` event to indicate the transition from OFF to ON and an `up` event to indicate the transition from ON to OFF.<br/>
    By specifying in the `modparam` parameter, the modifier can triger `doubleclick` and `singleclick` event to support double-click functionality, trigger a `longpress` event when held for an extended duration, or triger `follow_down` or `follow_up` events to indicate the passage of time after `down` or `up` events occur.<br/>
    Furthermore, it can be used to specify behaviors similar to key repetition. 
    
    This modifier can also be employed to enable binary functionality for [**Device Unit**](#device-unit) that inherently possess continuous quantities like analog axes.

- **`incdec`**<br/>
    This modifier is intended for application to [**Device Unit**](#device-unit)s that operate on relative values.
    It generates an `increment` event when the change is positive and a `decrement` event when it is negative.
    The [**Event Value**](/guide/event-action-mapping#event) represents the absolute value of the [**Device Unit**](#device-unit).

The `modifiers` parameter of [`mapper.device()`](/libs/mapper/mapper_device) specifies the definition of modifiers to apply to each **Event Modifier** using an array, similar to [this code snippet](https://github.com/opiopan/fsmapper/blob/v0.9.1/samples/practical/g1000.lua#L10-L19) in the [sample script](/samples/g1000).<br/>
Here are explanations for each key-value pair within the table representing the definition of a modifier.

|Key|Type|Description|
|---|----|-----------|
|`name`|string|Specifies the name of the [**Device Unit**](#device-unit) targeted by the modifier.<br/>If an [**Event Modifier**](/guide/device/#event-modifier) is simultaneously defined for the class associated with the [**Device Unit**](/guide/device#device-unit) specified by this parameter, the [**Event Modifier**](/guide/device/#event-modifier) specified by the `name` takes precedence.
|`class`|string|Specifies when applying the same modifier to multiple [**Device Unit**](#device-unit)s with similar characteristics.<br/>It specifies one of the following: `binary` for units with binary value ranges, `absolute` for units with absolute value ranges, or `relative` for units with relative value ranges.
|`modtype`|string|Modifier type.<br/>It specifies eather of `raw`, `button`, or `incdec`
|`modparam`|table|Options specific to the modifier.<br/>For detailed information, refer to the [**Event Modifier Specification**](modifier).

## Event IDs Table
The necessary [**Event ID**](/guide/event-action-mapping#event)s of ***Input*** type [**Device Units**](#device-unit) required to define [**Event-Action mappings**](/guide/event-action-mapping) can be obtained by referencing the table returned by the [`Device:get_events()`](/libs/mapper/Device/Device-get_events) method.
This table stores [**Event ID**](/guide/event-action-mapping#event)s under the names `[Device-Unit-Name].[Event-name]`. <br/>
You can register [**Action**](/guide/event-action-mapping#action)s for [**Event**](/guide/event-action-mapping#event)s triggered by the [**Device Units**](#device-unit) as follows.

```lua {9,12}
device = Device{
    name = "SimHID G1000",
    type = "simhid",
    identifier = {path = 'COM3'},
    modifiers = {
        {class = "binary", modtype = "button"},
    }
}
events = device:get_events()
mapper:set_primary_mappings{
    {
        event = events.SW1.down,
        action = fs2020.mfwasm.rpn_executer('(>K:AP_MASTER)')
    }
}
```

## Output Unit IDs Table
You can alter the state of ***Output*** type [**Device Units**](#device-unit) using the [**native-actions**](/guide/event-action-mapping#action) returned by the [`Device:send()`](/libs/mapper/Device/Device-send) method or the [`Device:sender()`](/libs/mapper/Device/Device-sender) method.

Both methods require specifying an integer value, known as the **Output Unit ID**, to identify the [**Device Units**](#device-unit).
You can obtain the **Output Unit ID** by referencing the table returned by the [`Device.get_upstream_ids()`](/libs/mapper/Device/Device-get_upstream_ids) method, where the **Output Unit ID** is stored with the [**Device Units**](#device-unit) name as the key.

Here's how you can modify the state of Output type [**Device Units**](#device-unit).

```lua {6,7}
device = Device{
    name = "SimHID pannel",
    type = "simhid",
    identifier = {path = 'COM4'},
}
ids = device:get_upstream_ids()
device:send(ids.LandingGearLeverIndicator, 1)
```

## Lifecycle of Device object
[`Device`](/libs/mapper/Device) object handling for device operations remains active until either [`Device:close()`](/libs/mapper/Device/Device-close) is called or until the object is deleted by garbage collection (GC). 
This implies that during device manipulation and while needing to receive events from the device, precautions must be taken to prevent the [`Device`](/libs/mapper/Device) object from being targeted by GC.

Especially when dealing solely with ***Input*** type [**Device Units**](#device-unit), situations where objects or functions managed by fsmapper like [**Event-Action mappings**](/guide/event-action-mapping) or [**Views**](/guide/virtual_instrument_panel#view) bind to the [`Device`](/libs/mapper/Device) object are less likely.

This is why in the code samples provided in this documentation, the [`Device`](/libs/mapper/Device) object is bound to a global variable.

## Supporting Custom Devices
Creating a plugin driver module enables handling devices other than the built-in [**Device Types**](#device-type). For more details, refer to the [**Plugin SDK**](/sdk).