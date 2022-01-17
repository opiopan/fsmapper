fsmapper
===
fsmapper is a windows application to connect various input and output devices to filght simulator.<br>
Initially this software was planned to connect my DIY controller device, [SimHID G1000](https://github.com/opiopan/simhid-g1000), to FS2020.
But fsmapper is designed to handle common gaming controller device now.

fsmapper allow to:
- Invoke Microsoft FS2020 event, such as turning autopilot on, according to the operation of SimHID devices or USB gaming controller device
- Change location, size, and/or visibility of pop-out windows of FS2020 such as G1000 PFD according to the operation of SimHID device of USB gaming controller device
- Change the mapping rule between SimHID device or USB gaming controller device operation and FS2020 events when device is operated or aircraft is changed
- Remap SimHID device or USB gaming controller device operation to virtual joystick device operation using vJoy driver

The above functions are configuread flexibly by writing a Lua scrip.

## How to build
1. **Requirements**<br>
Make sure that the following softwares are installed in advance.
    - Visual Studio 2019 or 2022 whilch is instaled with folowing workloads:
        - Universal Windows Platform development
        - C++ desktop development
        - C++ game development
    - [Windows App SDK extension for Visual Studio (C++)](https://docs.microsoft.com/en-us/windows/apps/windows-app-sdk/downloads)
    - Flight Simulator 2020 SDK 

2. **Downloading Source Codes**<br>
    ```shell
    $ git clone --recursive https://github.com/opiopan/fsmapper.git
    ```

3. **Preparing dependent modules**<br>
    Downloading and compiling Lua source codes and downloading vJoySDK will be done by following step.<br>
    Note that this step and following steps must be run the terminal environment which is configured to compile x64 binary.
    ```shell
    $ cd fsmapper\modules
    $ .\prepare_modules.bat
    ```

4. **Compiling**
    ```shell
    $ cd ..\src
    $ msbuild /t:mockup /p:Configuration=Release

## Running fsmapper
The core module as DLL to handle device operation and mapping those event to the flight simulator driven by Lua script is almost implemeted. However the GUI for this software has not been implemented yet so far.<br>
I  made a mockup CLI module instead the GUI in order to test the core engine.<br>
This mockup utility can be launched with Lua script path as below.

```shell
$ src\x64\Release\testmock.exe samples\g1000.lua
```

## Convenient Software
The SimConnect SDK of Flight Simulator 2020 defines many [EVENT IDs](https://docs.flightsimulator.com/html/index.htm#t=Programming_Tools%2FSimVars%2FEvent_IDs.htm) to interact with the guages and panels of aircraft. 
For example, ```G1000_PFD_FLIGHTPLAN_BUTTON``` is defined for the FPL button on the PFD.<br>
It seems that outer software can invoke guages or panel operation by sending such events via SimConnect. However almost event case, Flight Simulator 2020 doesn't react even if the event are sent from outer software via SimConnect.<br>
[MobiFlight WASM Module](https://github.com/MobiFlight/MobiFlight-WASM-Module) solves this problem.
This WASM module allows to define user defined event and allows to invoke RPN script to operate cockpit items.<br>
I've installed this module into the ```community``` folder of Flight Simulator 2020.
And [sample scprits](samples) for fsmapper uses events defined by this WASM module.<br>
