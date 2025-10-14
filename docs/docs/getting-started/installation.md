---
sidebar_position: 1
---

# Installation

This section covers the steps for installing fsmapper.

:::info
fsmapper is released as open-source under the Apache-2.0 License, allowing free usage, including modifications to the source code. <br/>
For instructions on building and installing from the source code, please refer [**here**](https://github.com/opiopan/fsmapper#how-to-build-and-install).
:::

## Download zip package

import {Version} from '@site/.version';
export const LatestPackage = ({children}) => (
    <a href={Version.package}>{children}</a>
);

You can download <LatestPackage>the latest version (v{Version.text})</LatestPackage> of the installation package from <LatestPackage>**here**</LatestPackage>.
If you want to download a previous version, please access it [**here**](https://github.com/opiopan/fsmapper/releases).<br/>
Once you have downloaded the installation package (zip file), extract it to your preferred folder.

The file structure after extraction will look like the following.

```
fsmapper_x.x.x
├── fsmapper
│   ├── samples
│   │   └── sample script files are here
│   ├── sdk
│   │   └── plugin SDK files are here
│   ├── fsmapper.exe
│   └── supplementary files such as DLLs
└── README.txt
```

## Copy fsmapper folder
Copy the ```fsmapper``` folder directly from the package to the location where you want to install fsmapper. 
fsmapper can be launched from any folder it's placed in.

:::warning[Updating fsmapper may fail]

When you try to overwrite an existing fsmapper installation with a newer version, the copy process may fail because **`fsmapperhook.dll`** is still loaded by other processes.

This DLL is a *global message hook module*, and Windows keeps it injected into multiple processes while fsmapper is running. Although fsmapper uninstalls the hook on exit, the DLL might remain loaded until those processes finish processing Windows messages.

If you encounter an error such as *“Access denied”* or *“file in use”* during update, try one of the following:

- **Restart Windows**<br/>
    This is the simplest and most reliable solution.  
- **Manually unload the DLL**<br/>
    Run the following commands in a Command Prompt (Administrator):

   ```cmd
   tasklist /m fsmapperhook.dll
   ```

   This shows which processes have loaded the DLL. You can then terminate them (if safe) using:

   ```cmd
   taskkill /PID <pid> /F
   ```

   Usually, the remaining processes are system daemons such as svchost.exe, and killing them is harmless, Windows will automatically restart them.

This behavior is caused by how Windows manages global message hooks and is not specific to fsmapper.
:::

## Additional Softwares
While not mandatory for running fsmapper, it's highly recommended to additionally install the following two pieces of software. 
fsmapper provides users with more convenient features by integrating with these softwares.
Throughout this documentation, starting from the [Tutorial](tutorial), numerous script examples assume that these softwares are installed.

- **vJoy ([Download](https://sourceforge.net/projects/vjoystick))**<br/>
    vJoy is a device driver that functions as a virtual joystick. 
    fsmapper offers multiple functions to alter button status, axis positions, and POV settings of the vJoy device. <br/>
    Through the use of the vJoy device, fsmapper provides highly flexible configurability for human interface devices.

- **MobiFlight WASM Module ([GitHub](https://github.com/MobiFlight/MobiFlight-WASM-Module))**<br/>
    This WASM module, functioning as an add-on for Microsoft Flight Simulator 2020, 
    enables an external-process utility to execute arbitrary [**RPN script**](https://docs.flightsimulator.com/html/Additional_Information/Reverse_Polish_Notation.htm) and monitor changes in the aircraft's internal state stored in local variables like LVARs.
    fsmapper gains unrestricted access to retrieve and modify aircraft states by communicating with the MobiFlight WASM Module operating within the MSFS process.<br/>
    The easiest way to install the MobiFlight WASM Module is by downloading and installing the **MobiFlight Connector** from [here](https://www.mobiflight.com/en/download.html).

:::info
The vanilla fsmapper communicates with Microsoft Flight Simurator 2020 through the SimConnect API, which allows an external process to access only
[**SimVars**](https://docs.flightsimulator.com/html/Programming_Tools/SimVars/Simulation_Variables.htm) and
[**Event IDs**](https://docs.flightsimulator.com/html/Programming_Tools/Event_IDs/Event_IDs.htm). 
However, this access isn't sufficient to control all operable objects in a cockpit or retrieve all gauge statuses to display on another DIY gauge or virtual instrument panel.<br/>
To control and access all cockpit gauges, accessing local variables within an aircraft module is necessary, and the [**Gauge API**](https://docs.flightsimulator.com/html/Programming_Tools/WASM/Gauge_API/Gauge_API.htm) serves this purpose. Regrettably, this API can only be used within the WASM module.<br/>
The MobiFlight WASM Module addresses this limitation, enabling fsmapper to access all local variables by communicating with this WASM module.
:::
