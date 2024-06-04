@echo off

echo ===================================================
echo  module preparing
echo ===================================================
cd modules
call .\prepare_modules.bat
call :SuccessOrDie

echo ===================================================
echo  compiling
echo ===================================================
cd ..\src
nuget restore fsmapper.sln
call :SuccessOrDie
msbuild /p:Configuration=Release
call :SuccessOrDie

echo ===================================================
echo  build deployable package
echo ===================================================
cd ..\deploy
call .\deploy.bat
call :SuccessOrDie
cd ..

:SuccessOrDie
if not %errorlevel% == 0 (
    echo [ERROR] :P
    cd ..
    exit 1
)
exit /b 0
