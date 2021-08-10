set VERSION=5.4.3

curl -s https://www.lua.org/ftp/lua-%VERSION%.tar.gz | tar -zxvf -
cd lua-%VERSION%\src
cl /MD /O2 /c /DLUA_BUILD_AS_DLL *.c
ren lua.obj lua.o
ren luac.obj luac.o
link /DLL /IMPLIB:lua%VERSION%.lib /OUT:lua%VERSION%.dll *.obj
link /OUT:lua.exe lua.o lua%VERSION%.lib
lib /OUT:lua%VERSION%-static.lib *.obj
link /OUT:luac.exe luac.o lua%VERSION%-static.lib
cd ..\..