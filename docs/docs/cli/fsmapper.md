---
sidebar_position: 1
---

# fsmapper

This command is the fsmapper executable itself and launches the fsmapper GUI.

fsmapper is designed to allow only a single instance to run on the system.
If fsmapper is already running when this command is executed, the existing fsmapper window will be brought to the foreground, and the command will immediately exit.
If the window is minimized at that time, it will be restored to its previous size.

These behaviors can be modified by specifying the following parameters.

## Syntax
```
fsmapper [/i] [<Script-Path>]
```

## Parameters

|Parameter|Description|
|---------|-----------|
|`/i`|Launches fsmapper with the window minimized.<br/><br/>If fsmapper is already running, this option prevents the default behavior of bringing the window to the foreground. However, it does not actively minimize the window, its size, position, and Z-order remain unchanged.
|`<Script-Path>`|Specifies the path to a Lua script to be executed at fsmapper startup. The path can be either relative or absolute. If fsmapper is already running, the currently executing script will be stopped, and the script specified by this parameter will be executed.<br/><br/>If this parameter is not specified, the script that was running when fsmapper was last closed will be executed.<br/><br/>Even if **Auto run** is disabled in the **Settings** page, the script specified with this parameter will still be executed at startup.

## Remarks
* If unsupported options or extra parameters are specified, the command will not result in an error. These specifications will simply be ignored.
* If a path that does not exist or one with insufficient access rights is specified for `<Script-Path>`, the command will not result in an error. In this case, the error reason will be displayed in the fsmapper **Message Console** page.