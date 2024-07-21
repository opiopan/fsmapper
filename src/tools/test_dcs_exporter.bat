@echo off
if "%~1" == "" (
    echo ERROR: No DCS path is specified
    exit /b 1
)

set tooldir=%~dp0
set luadir=%tooldir%..\..\modules\lua-5.4.6\src\
set exporterdir=%tooldir%..\core\dcs-exporter\
set dcsdir=%~1

REM %luadir%lua.exe %tooldir%dcs_mock.lua %exporterdir% %~1
%dcsdir%\bin-mt\luae.exe %tooldir%dcs_mock.lua %exporterdir% %dcsdir%
