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

<p align="center">
<img alt="description" src="https://raw.githubusercontent.com/wiki/opiopan/fsmapper/images/fsmapper.png" width=700>
</p>

## How to build and install
1. **Requirements**<br>
Make sure that the following softwares are installed in advance.
    - Visual Studio 2019 or 2022 whilch is instaled with folowing workloads:
        - Universal Windows Platform development
        - C++ desktop development
        - C++ game development
    - [Windows App SDK extension for Visual Studio (C++)](https://docs.microsoft.com/en-us/windows/apps/windows-app-sdk/downloads)
    - Flight Simulator 2020 SDK 
    - [nuget.exe](https://www.nuget.org/downloads) should be placed at the folder which is indicated by PATH environment variable.

2. **Running a console**<br>
Run ```cmd.exe``` or ```PowerShell.exe``` on any console. Note that environment variables are set to complie x64 binaries.<br>
The one of easiest way is using the following shortcut made when Visual Studio was installed.
    - x64 Native Tools Command Prompt for VS 2022
    - x64 Native Tools Command Prompt for VS 2019


2. **Downloading source codes**<br>
    ```shell
    $ git clone --recursive https://github.com/opiopan/fsmapper.git
    ```

3. **Preparing dependent modules**<br>
    Downloading and compiling Lua source codes and downloading vJoySDK will be done by following step.<br>
    ```shell
    $ cd fsmapper\modules
    $ .\prepare_modules.bat
    ```

4. **Compiling**
    ```shell
    $ cd ..\src
    $ nuget restore fsmapper.sln
    $ msbuild /p:Configuration=Release
    ```
5. **Making deployable package**
    ```shell
    $ cd ..\deploy
    $ .\deploy.bat
    ```

6. **Installing**<br>
    Copy fsmmapper folder to any folder you want.
    ```shell
    $ xcopy fsmapper <DESTINATION_FOLDER_PATH>
    ```
## How to use fsmapper

### Configuration file = Lua scrip
To briefly describe the function of fsmapper is that waiting event such as position change of the joystick axis then invoking the action corrensponds to the occured event.<br> 
The rule of mapping between the events and the actions is described by [Lua 5.4](https://www.lua.org/manual/5.4/manual.html) script.
The executable file of fsmapper, fsmapper.exe, provides GUI. However fsmapper has no capability to edit the event-action mapping rules. It just can behave like a dashboard to show the condition and state of event-action mapping process.<br>
Therefore, the first thing you have to do is writing a configuration file by Lua script.

I haven't writen the specification of this configuration file yet but some examples are pleaced [here](samples).

### Running fsmapper
The first time of launching fsmapper, you need to specify the configuration Lua script file by press the open button located to the left of the Run button. After that start event-action mapping process by press the Run button.<br>
At the second and subsequent famapper launches, same script file will be executed automatically.

When event-action mapping process is aborted due to error, the status indicater placed at the top of window blinks red.<br>
In this case, refere the error message for details by acessing the message console page as below.

<p align="center">
<img alt="message console" src="https://raw.githubusercontent.com/wiki/opiopan/fsmapper/images/console.png" width=600>
</p>

### Captured window
fsmapper has a function to controll visibility and position of any window ownd by other process. This function is designed to enable a display as multipurpose, especially to handle fs2020 puped out instrument window like [this movie](https://raw.githubusercontent.com/wiki/opiopan/simhid-g1000/images/movie.gif).

Those windows controlled by fsmapper are called "captured window" and those are specified by ```mapper.captured_window()``` function in Lua script.<br>
Unfortunately, FS2020 SDK does not allow to controll poped out window and does not provide a way to recognize the difference between poped out windows. From fsmapper point of view, all FS2020 poped out windows looks same.<br>
So Lua script defines just placefolder, You need to specify which actual window corresponds to captured window definition at run time as below.

<p align="center">
<img alt="description" src="https://raw.githubusercontent.com/wiki/opiopan/fsmapper/images/captured_window.gif">
</p>

## Convenient Software
The SimConnect SDK of Flight Simulator 2020 defines many [EVENT IDs](https://docs.flightsimulator.com/html/index.htm#t=Programming_Tools%2FSimVars%2FEvent_IDs.htm) to interact with the guages and panels of aircraft. 
For example, ```G1000_PFD_FLIGHTPLAN_BUTTON``` is defined for the FPL button on the PFD.<br>
It seems that outer software can invoke guages or panel operation by sending such events via SimConnect. However almost event case, Flight Simulator 2020 doesn't react even if the event are sent from outer software via SimConnect.<br>
[MobiFlight WASM Module](https://github.com/MobiFlight/MobiFlight-WASM-Module) solves this problem.
This WASM module allows to define user defined event and allows to invoke RPN script to operate cockpit items.<br>
I've installed this module into the ```community``` folder of Flight Simulator 2020.
And [sample scprits](samples) for fsmapper uses events defined by this WASM module.<br>
