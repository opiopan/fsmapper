---
sidebar_position: 1
id: mapper_index
---

# mapper library
fsmapper basic library.

## Variables
|Name|Description|
|-|-|
|[```mapper.script_path```](/libs/mapper/mapper_script_path)|Path of the configuration script|
|[```mapper.script_dir```](/libs/mapper/mapper_script_dir)|Folder path where the configuration script is located|
|[```mapper.events```](/libs/mapper/mapper_events)|System event table|

## Functions
|Name|Description|
|-|-|
|[```mapper.print()```](/libs/mapper/mapper_print)|Print a message|
|[```mapper.abort()```](/libs/mapper/mapper_abort)|Abort processing|
|[```mapper.delay()```](/libs/mapper/mapper_delay)|Deferred function execution|
|[```mapper.register_event()```](/libs/mapper/mapper_register_event)|Register an event|
|[```mapper.unregister_event()```](/libs/mapper/mapper_unregister_event)|Unregister an event|
|[```mapper.get_event_name()```](/libs/mapper/mapper_get_event_name)|Get the name assinged to an event|
|[```mapper.raise_event()```](/libs/mapper/mapper_raise_event)|Raise an event|
|[```mapper.set_primary_mappings()```](/libs/mapper/mapper_set_primary_mappings)|Set primary Event-Action mapping definitions|
|[```mapper.add_primary_mappings()```](/libs/mapper/mapper_add_primary_mappings)|Add primary Event-Action mapping definitions|
|[```mapper.set_secondary_mappings()```](/libs/mapper/mapper_set_secondary_mappings)|Set secondary Event-Action mapping definitions|
|[```mapper.add_secondary_mappings()```](/libs/mapper/mapper_add_secondary_mappings)|Add secondary Event-Action mapping definitions|
|[```mapper.device()```](/libs/mapper/mapper_device)|Open a device|
|[```mapper.viewport()```](/libs/mapper/mapper_viewport)|Register a viewport|
|[```mapper.view_elements.operable_area()```](/libs/mapper/mapper_view_elements_operable_area)|Create a OperableArea view element object|
|[```mapper.view_elements.canvas()```](/libs/mapper/mapper_view_elements_canvas)|Create a Canvas view element object|
|[```mapper.view_elements.captured_window()```](/libs/mapper/mapper_view_elements_captured_window)|Create a CapturedWindow view element object|
|[```mapper.start_viewports()```](/libs/mapper/mapper_start_viewports)|Start all viewports|
|[```mapper.stop_viewports()```](/libs/mapper/mapper_stop_viewports)|Stop all viewports|
|[```mapper.reset_viewports()```](/libs/mapper/mapper_reset_viewports)|Stop all viewports then unregister all viewports|
|[```mapper.virtual_joystick()```](/libs/mapper/mapper_virtual_joystick)|Create vJoy feeder object|
|[```mapper.keystroke()```](/libs/mapper/mapper_keystroke)|Create Keystroke object for keybord emulation|

## Objects
|Name|Description|
|-|-|
|[```Device```](/libs/mapper/Device)|Object representing a device|
|[```Viewport```](/libs/mapper/Viewport)|Object representing a viewport|
|[```OperableArea```](/libs/mapper/OperableArea)|Object Representing a operable area on a view|
|[```Canvas```](/libs/mapper/Canvas)|Object Representing a drawable area on a view|
|[```CapturedWindow```](/libs/mapper/CapturedWindow)|Object representing a captured window placed on a view|
|[```vJoy```](/libs/mapper/vJoy)|Object representing a virtual joystick|
|[```vJoyUnit```](/libs/mapper/vJoyUnit)|Object representing an operable unit of the virtual joystick|
|[```Keystroke```](/libs/mapper/Keystroke)|Object Representing a keystroke sequense to emulate keyboard|

## User Defined Functions
|Name|Description|
|-|-|
|[```ACTION()```](/libs/mapper/ACTION)|Imprementation of an action by user|
|[```RENDER()```](/libs/mapper/RENDER)|Imprementation of a renderer for a Canvas view element by user|
