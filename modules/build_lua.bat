curl -s https://www.lua.org/ftp/lua-5.4.3.tar.gz | tar -zxvf -
cd lua-5.4.3\src
cl /MD /O2 /c /DLUA_BUILD_AS_DLL *.c
ren lua.obj lua.o
ren luac.obj luac.o
link /DLL /IMPLIB:lua5.3.5.lib /OUT:lua5.3.5.dll *.obj
link /OUT:lua.exe lua.o lua5.3.5.lib
lib /OUT:lua5.3.5-static.lib *.obj
link /OUT:luac.exe luac.o lua5.3.5-static.lib
cd ..\..