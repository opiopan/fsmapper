@powershell -NoProfile -ExecutionPolicy Unrestricted "$s=[scriptblock]::create((gc \"%~f0\"|?{$_.readcount -gt 1})-join\"`n\");&$s" %*&goto:eof

$lua_version = "5.4.6"
$vjoysdk = "vJoy218SDK-291116.zip"
$vjoysdk_dest = "vJoySDK"
$vjoysdk_header = "vjoyinterface.h"
$vjoysdk_header_path = $vjoysdk_dest + "\SDK\inc\" + $vjoysdk_header
$lua_dir = "lua-" + $lua_version
if (Test-Path $lua_dir){
    Remove-Item $lua_dir -Recurse
}
$download_cmd = "curl.exe https://www.lua.org/ftp/" + $lua_dir + ".tar.gz | tar.exe -zxvf -"
cmd /c $download_cmd
if (Test-Path $lua_dir){
    Set-Location lua-$lua_version\src
    cl /MT /O2 /c /DLUA_BUILD_AS_DLL *.c
    Rename-Item -Path lua.obj lua.o
    Rename-Item -Path luac.obj luac.o
    link /DLL /IMPLIB:lua$lua_version.lib /OUT:lua$lua_version.dll *.obj
    link /OUT:lua.exe lua.o lua$lua_version.lib
    lib /OUT:lua$lua_version-static.lib *.obj
    link /OUT:luac.exe luac.o lua$lua_version-static.lib
    Set-Location ..\..
}

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
