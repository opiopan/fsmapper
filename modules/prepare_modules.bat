@powershell -NoProfile -ExecutionPolicy Unrestricted "$s=[scriptblock]::create((gc \"%~f0\"|?{$_.readcount -gt 1})-join\"`n\");&$s" %*&goto:eof

$lua_version = "5.4.7"
$dcs_lua_version = "5.1.5"
$vjoysdk = "vJoy218SDK-291116.zip"
$vjoysdk_dest = "vJoySDK"
$vjoysdk_header = "vjoyinterface.h"
$vjoysdk_header_path = $vjoysdk_dest + "\SDK\inc\" + $vjoysdk_header

$lua_dir = "lua-" + $lua_version
if (Test-Path $lua_dir){
    Remove-Item $lua_dir -Recurse
}
if (Test-Path lua-5.4){
    Remove-Item lua-5.4 -Recurse
}
$download_cmd = "curl.exe https://www.lua.org/ftp/" + $lua_dir + ".tar.gz | tar.exe -zxvf -"
cmd /c $download_cmd
if (Test-Path $lua_dir){
    Set-Location lua-$lua_version\src
    cl /MT /O2 /c /DLUA_BUILD_AS_DLL *.c
    Rename-Item -Path lua.obj lua.o
    Rename-Item -Path luac.obj luac.o
    lib /OUT:lua5.4-static.lib *.obj
    link /OUT:luac.exe luac.o lua5.4-static.lib
    link /DLL /IMPLIB:lua5.4.lib /OUT:lua5.4.dll *.obj
    link /OUT:lua.exe lua.o lua5.4.lib
    Set-Location ..\..
}
Rename-Item $lua_dir lua-5.4

$dcs_lua_dir = "lua-" + $dcs_lua_version
if (Test-Path $dcs_lua_dir){
    Remove-Item $dcs_lua_dir -Recurse
}
if (Test-Path lua-5.1){
    Remove-Item lua-5.1 -Recurse
}
$download_cmd = "curl.exe https://www.lua.org/ftp/" + $dcs_lua_dir + ".tar.gz | tar.exe -zxvf -"
cmd /c $download_cmd
if (Test-Path $dcs_lua_dir){
    Set-Location lua-$dcs_lua_version\src
    # Write-Output "#undef LUA_COMPAT_OPENLIB" | Add-Content luaconf.h -Encoding UTF8
    lib /DEF:..\..\lua-builtin-dcs\lua.def /MACHINE:x64
    cl /MT /O2 /c  *.c
    Rename-Item -Path lua.obj lua.o
    Rename-Item -Path luac.obj luac.o
    lib /OUT:lua5.1-static.lib *.obj
    Set-Location ..\..
}
Rename-Item $dcs_lua_dir lua-5.1

if (!(Test-Path $vjoysdk_dest)){
    New-Item $vjoysdk_dest -ItemType Directory
}
curl.exe -L https://sourceforge.net/projects/vjoystick/files/Beta%202.x/SDK/$vjoysdk/download --output $vjoysdk
Expand-Archive -Path $vjoysdk -DestinationPath $vjoysdk_dest -Force
Remove-Item $vjoysdk
$backup = $vjoysdk_header + ".bak"
$backup_path = $vjoysdk_header_path + ".bak"
if (Test-Path $backup_path){
    Remove-Item $backup_path
}
Rename-Item -Path $vjoysdk_header_path -NewName $backup
get-content -Encoding Ascii $backup_path | Out-File -Encoding UTF8 $vjoysdk_header_path
