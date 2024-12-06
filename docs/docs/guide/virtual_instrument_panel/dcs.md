---
sidebar_position: 2
---

# How to integrate DCS Instruments
This page explains how to integrate exported instruments, such as MFCDs from DCS World, into fsmapper’s virtual instrument panel. 
For a comprehensive overview of the concepts and features provided by fsmapper for virtual instrument panels, refer to the [**Virtual Instrument Panel**](/guide/virtual_instrument_panel/) as well.

## Exporting DCS Instruments
Unlike Microsoft Flight Simulator, DCS World does not support popping out instruments into separate windows.
DCS World is designed to render graphics on a single Direct3D surface, which is displayed in a single window.
However, in a multiple display setup, the surface size can be extended beyond the dimensions of a single display, allowing it to span across multiple displays.
Additionally, DCS World allows specifying arbitrary positions and sizes on the surface to display instruments such as MFCDs.
This functionality enables exporting instruments to displays other than the main display.

These configurations are performed by creating custom Lua files tailored to the user’s environment. For more details, please refer to [this guide](https://forum.dcs.world/topic/258724-how-to-multi-monitor-mfcd-display-export-set-up-guide-nov-2024-updated/).

## Static Integration
The simplest method to integrate the instruments exported by DCS World with fsmapper’s virtual instrument panel is to treat the position and size of the exported instruments as static,
while placing custom-drawn instruments or buttons in the remaining areas of the panel.
This approach is similar to the one used by tools like [Helios](https://github.com/HeliosVirtualCockpit/Helios).

For example, you can display a bezel-like image that only includes buttons surrounding the exported MFCD area, making the MFCD portion itself transparent.
Touch interactions on the buttons can then trigger interactions with DCS by using [these functions](/guide/dcs/).

While this is a common method for constructing virtual instrument panels for DCS World, it lacks flexibility. It restricts the use of specific sub-display regions to fixed purposes. For instance, you cannot dynamically reconfigure the display to split the sub-display between the left and right MFCDs at one moment and then replace the left MFCD with a center MFCD at another.

## Dynamic Integration
fsmapper provides the [`WindowImageStreamer`](/libs/mapper/WindowImageStreamer/) object and the [`CapturedImage`](/libs/mapper/CapturedImage/) view element to integrate exported DCS instruments into a virtual instrument panel with [the same level of flexibility as in Microsoft Flight Simulator](/guide/virtual_instrument_panel#components-for-virtual-instrument-panel).

Here, we will use an example scenario where a Lua script for multi-display configuration has been registered in DCS.

```lua title='myconfig.lua'
_  = function(p) return p; end;
name = _('My config');
Description = 'two monitor configuration'

Viewports = {
     Center = {
          x = 0, y = 0, width = 3840, height = 2160,
          viewDx = 0, viewDy = 0, aspect = 3840 / 2160,
     },
}
UIMainView = Viewports.Center
GU_MAIN_VIEWPORT = Viewports.Center

LEFT_MFCD = {x = 0, y = 2160, width = 1024, height = 1024}
RIGHT_MFCD = {x = 1024, y = 2160, width = 1024, height = 1024}

function reconfigure_for_unit(unit_type)
    if unit_type == 'F-16C_50' then
        EHSI = {x = 2048, y = 2160, width = 800, height = 800}
    else if unit_type == 'FA-18C_hornet' then
        CENTER_MFCD = {x = 2048, y = 2160, width = 1024, height = 1024}
    end
end
```

### WindowImageStreamer object
[`WindowImageStreamer`](/libs/mapper/WindowImageStreamer/) object continuously captures and retains the display content of another process’s window in real time using the [Windows.Graphics.Capture](https://learn.microsoft.com/en-us/uwp/api/windows.graphics.capture?view=winrt-26100) API.

To dynamically integrate instruments exported by DCS World, the first step is to create a [`WindowImageStreamer`](/libs/mapper/WindowImageStreamer/) object using [`mapper.window_image_streamer()`](/libs/mapper/mapper_window_image_streamer/) to capture the display content of DCS World’s main window.
This function requires specifying a window title to identify the target window.
However, since the default value is `Digital Combat Simulation`, there is no need to provide this parameter when targeting DCS World.

```lua
-- Create a WindowImageStreamer object for DCS
local dcs_streamer = mapper.window_image_streamer()
```

fsmapper periodically scans for a window with the title specified during the creation of the [`WindowImageStreamer`](/libs/mapper/WindowImageStreamer/) object and automatically starts image capture when the window is found.
However, if the automatic capture does not work since the specified window title is incorrect, the user can manually specify the target window.
For details on the manual specification process, refer to the [`Handling Popped out Windows`](/guide/virtual_instrument_panel/#handle-popped-out-windows).

### CapturedImage view element
[`CapturedImage`](/libs/mapper/CapturedImage) object is a view element object used to display a partial rectangular region of the window image captured by the [`WindowImageStreamer`](/libs/mapper/WindowImageStreamer/) in a view.

[`CapturedImage`](/libs/mapper/CapturedImage) objects are created using the [`create_view_element()`](/libs/mapper/WindowImageStreamer/WindowImageStreamer-create_view_element/) method of a [`WindowImageStreamer`](/libs/mapper/WindowImageStreamer/) object. 
While only one [`WindowImageStreamer`](/libs/mapper/WindowImageStreamer/) object is typically created per DCS process, it is common practice to create a separate [`CapturedImage`](/libs/mapper/CapturedImage) object for each exported instrument.

Below is an example of registering two views in a viewport, one containing the left MFCD and right MFCD, and the other containing the left MFCD and center MFCD.

```lua
-- Create a viewport on the display 2
local viewport = mapper.viewport{
    name = 'DCS F/A-18C',
    displayno = 2,
}

-- Create view elements for each exported instruments
local left_mfcd = dcs_streamer:create_view_element{
    x = 0, y = 2160, width = 1024, height = 1024
}
local right_mfcd = dcs_streamer:create_view_element{
    x = 1024, y = 2160, width = 1024, height = 1024
}
local right_mfcd = dcs_streamer:create_view_element{
    x = 2048, y = 2160, width = 1024, height = 1024
}

-- Register a view containing the left MFCD and right MFCD
viewport:register_view{
    name = 'view 1',
    background = graphics.bitmap('assets/two_mfcd.png'),
    elements = {
        {object=left_mfcd, x=0.05, y=0, width=0.4, height=1},
        {object=right_mfcd, x=0.55, y=0, width=0.4, height=1},
    },
}

-- Register a view containing the left MFCD and center MFCD
viewport:register_view{
    name = 'view 2',
    background = graphics.bitmap('assets/two_mfcd.png'),
    elements = {
        {object=left_mfcd, x=0.05, y=0, width=0.4, height=1},
        {object=center_mfcd, x=0.55, y=0, width=0.4, height=1},
    },
}
```

### Helper Function
Defining the following function makes it easier to generate [`CapturedImage`](/libs/mapper/CapturedImage/) objects.

```lua
function exported_instruments(path, unit)
    local env = {}
    setmetatable(env, {__index=_ENV})
    local chunk = loadfile(path, 'bt', env)
    chunk()
    if env.reconfigure_for_unit then
        env.reconfigure_for_unit(unit)
    end
    return env
end
```

Using this function, the view element creation part from the previous section can be rewritten as follows.
By retrieving the coordinates and sizes of the exported instruments from the DCS configuration file, it helps reduce errors.

```lua
-- Parse the monitor configuration for DCS
local monitor_config = mapper.saved_games_dir .. [[\DCS\Config\MonitorSetup\myconfig.lua]]
local instruments = exported_instruments(monitor_config, 'FA-18C_hornet')

-- Create view elements for each exported instruments
local left_mfcd = dcs_streamer:create_view_element(instruments.LEFT_MFCD)
local right_mfcd = dcs_streamer:create_view_element(instruments.RIGHT_MFCD)
local right_mfcd = dcs_streamer:create_view_element(instruments.CENTER_MFCD)
```
