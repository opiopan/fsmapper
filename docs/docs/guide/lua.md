---
sidebar_position: 1
---

# Lua in fsmapper

fsmapper incorporates a Lua script interpreter. 
This section provides Lua-related information necessary for writing Lua scripts as configuration files for fsmapper. 
However, it does not delve into the specifics of Lua language usage, so please refer to the [**Lua 5.4 Reference Manual**](https://www.lua.org/manual/5.4/) for details on Lua itself.

## Lua Version
|Verion|
|------|
|5.4.7 |

## Available Lua Standard Libraries

The following libraries from the Lua standard library are enabled by default.

|Name| Description|
|----|------------|
|[basic](https://www.lua.org/manual/5.4/manual.html#6.1)|Basic functions|
|[package](https://www.lua.org/manual/5.4/manual.html#6.3)|Basic facilities for loading modules|
|[string](https://www.lua.org/manual/5.4/manual.html#6.4)|String manipulation|
|[table](https://www.lua.org/manual/5.4/manual.html#6.6)|Table manipulation|
|[math](https://www.lua.org/manual/5.4/manual.html#6.7)|Mathematical functions|

If you want to use libraries other than those listed above in the configuration file or if you want to disable certain libraries for memory working set efficiency, you can change these settings on the settings page from the default configuration.

![How to specify which library is enabled](images/select_stdlibs.png)
