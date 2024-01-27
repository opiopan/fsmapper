---
id: inputemu_index
sidebar_position: 1
---

# Changing Input Device Characteristics
To modify the characteristics of input devices, fsmapper provides functionalities for emulating joysticks and keyboard inputs.
Users can employ these functions within [**Event-Action mappings**](/guide/event-action-mapping) to reflect input from devices into the state of virtual joystick devices or generate emulated keystroke sequences.

## Joystick emulation
fsmapper emulates joystick devices using [**vJoy**](https://sourceforge.net/projects/vjoystick/) devices.
To utilize this feature, [**vJoy**](https://sourceforge.net/projects/vjoystick/) devices must be installed and the necessary axes and buttons need to be enabled on the device. Please ensure the correct configuration of [**vJoy**](https://sourceforge.net/projects/vjoystick/) devices using `Configure vJoy.exe` beforehand.

### vJoy Object
The [`vJoy`](/libs/mapper/vJoy) object represents a vJoy device.
You can create the [`vJoy`](/libs/mapper/vJoy) object by specifying the vJoy device number in [`mapper.virtual_joystick()`](/libs/mapper/mapper_virtual_joystick).

```lua
local vjoy = mapper.virtual_joystick(1)
```

### vJoyUnit Object
The [`vJoyUnit`](/libs/mapper/vJoyUnit) object represents individual axes, buttons, and POVs of a vJoy device.
By setting values for vJoyUnit, you can change the state of the vJoy device and emulate joystick inputs.

The [vJoyUnit](/libs/mapper/vJoyUnit) object can be created by specifying a value that identifies the unit in the following methods of the [vJoy](/libs/mapper/vJoy) object.

|Unit Type|Method|Argument|
|---------|------|--------|
|Axis|[`get_axis()`](/libs/mapper/vJoy/vJoy-get_axis)|Specify one of the following axis names: `'x'`, `'y'`, `'z'`, `'rx'`, `'ry'`, `'rz'`, `'slider1'`, `'slider2'`.
|Button|[`get_button()`](/libs/mapper/vJoy/vJoy-get_button)|Specify a button number starting from `1`.
|POV|[`get_pov()`](/libs/mapper/vJoy/vJoy-get_pov)|Specify a POV number starting from `1`.

The setting of [`vJoyUnit`](/libs/mapper/vJoyUnit) values can be performed using the [`vJoyUnit:set_value()`](/libs/mapper/vJoyUnit/vJoyUnit-set_value) or the [**native-action**](/guide/event-action-mapping#action) returned by [`vJoyUnit:value_setter()`](/libs/mapper/vJoyUnit/vJoyUnit-value_setter).
The values that can be configured for each unit are as follows.

|Unit Type|Values|
|---------|------|
|Axis|-50000 to 50000
|Button|1 when the button is pressed, 0 when released
|POV|The angle in degrees.

Here's an example script that toggles vJoy device buttons 1 through 4 sequentially each time button 1 on the USB gaming device is pressed:

```lua
-- Open a USB gaming device
usb_dev = mapper.device{
    name = 'USB gaming device',
    type = 'dinput',
    identifier = {index = 1},　-- Shoud be modified this line to suit your environment
    modifiers = {
        {class = 'binary', modtype = 'button'},
    }
}
local events = device:get_events()

-- Open a vJoy device
local vjoy = mapper.virtual_joystick(1)
local vjoy_buttons = {
    vjoy:get_button(1),
    vjoy:get_button(2),
    vjoy:get_button(3),
    vjoy:get_button(4),
}

-- State transition context
local current = 1
vjoy_buttons[current]:set_value(1)

-- Regsiter a Event-Action mapping
mapper.set_primary_mappings{
    {event=events.button1.down, action=fucntion
        vjoy_buttons[current]:set_value(0)
        current = current < 4 and current + 1 or 1
        vjoy_buttons[current]:set_value(1)
    end}
}
```

## Keystroke emulation
In fsmapper, keyboard operations can be emulated using [`Keystroke`](/libs/mapper/Keystroke) objects, representing any keystroke sequence.
To create a Keystroke object, specify the name of a virtual key from the [**Virtual-Key Codes and Names**](keycodes) list in [`mapper.keystroke()`](/libs/mapper/mapper_keystroke).

The following generates an object representing a keystroke sequence of `[A]`, `[B]`, and `[C]` occurring sequentially.
```lua
local seq1 = mapper.keystroke{codes={'A', 'B', 'C'}}
```

To specify simultaneous pressing of modifier keys like `[Ctrl]` or `[Shift]`, use the following notation.
```lua
local seq2 = mapper.keystroke{codes={'A', 'B', 'C'}, modifiers={'VK_LSHIFT', 'VK_LMENU'}}
```

The [`Keystroke`](/libs/mapper/Keystroke) object supports the `__add` method of the metatable, allowing the generation of new sequences by concatenating sequences using the `+` operator.
This enables the creation of sequences with different modifier keys as shown below.
```lua
local seq3 = mapper.keystroke{codes={'VK_F12'}, modifiers={'VK_RCONTROL'}}
local seq4 = seq3 + mapper.keystroke{codes={'H', 'J', 'K', 'L'}}
```

You can also customize the duration of key press and the interval between keystrokes in milliseconds.
```lua
-- You can customize the timing parameters when the objec is generated
local seq5 = mapper.keystroke{codes={'H', 'E', 'L', 'L', 'O'}, duration=100, interval=100}

-- You can also customize after the object generation
seq4.duration = 50
seq4.interval = 500

```

:::warning Note
Due to `duration` and `interval` being attributes held on a per [`Keystroke`](/libs/mapper/Keystroke) basis, be careful when concatenating with the '+' operator. The 'duration' and 'interval' of the new object will assume the values of the left-hand side object.
:::

To actually generate the defined keystroke sequence, use [`Keystroke():synthesize()`](/libs/mapper/Keystroke/Keystroke-synthesize) or the native-action returned by [`Keystroke():synthesizer()`](/libs/mapper/Keystroke/Keystroke-synthesizer).

```lua
-- Open a USB gaming device
usb_dev = mapper.device{
    name = 'USB gaming device',
    type = 'dinput',
    identifier = {index = 1},　-- Shoud be modified this line to suit your environment
    modifiers = {
        {class = 'binary', modtype = 'button'},
    }
}
local events = device:get_events()

-- Register Event-Action mappings to synthesize keystrokes
mapper.set_primary_mappings{
    {event=events.butoon1.down, action=mapper.keystroke{codes='A'}:synthesizer()},
    {event=events.butoon1.up, action=mapper.keystroke{codes='A', modfiers='VK_LCONTROL'}:synthesizer()},
}
```