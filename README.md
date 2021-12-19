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

## Building fsmapper
1. **Requirements**<br>
Visual Studio 2019 or later must be installed.

2. **Downloading Source Codes**<br>
    ```shell
    $ git clone --recursive https://github.com/opiopan/fsmapper.git
    ```

3. **Preparing dependent modules**<br>
    Downloading and compiling Lua source codes and downloading vJoySDK will be done by following step.
    ```shell
    $ cd fsmapper\modules
    $ .\prepare_modules.bat
    ```

4. **Compiling**
    ```shell
    $ cd ..\src
    $ msbuild /p:Configuration=Release

## Running fsmapper
The core module as DLL to handle device operation and mapping those event to the flight simulator driven by Lua script is almost implemeted. However the GUI for this software has not been implemented yet so far.<br>
I  made a mockup CLI module instead the GUI in order to test the core engine.<br>
This mockup utility can be launched with Lua script path as below.

```shell
$ src\x64\Release\testmock.exe samples\g1000.lua
```