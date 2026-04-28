@echo off
setlocal EnableDelayedExpansion
set "PD=%~dp0"
set "DLL="

if exist "%PD%..\Debug\rps2_tests.dll" set "DLL=%PD%..\Debug\rps2_tests.dll"
if exist "%PD%..\x64\Debug\rps2_tests.dll" if "!DLL!"=="" set "DLL=%PD%..\x64\Debug\rps2_tests.dll"
if exist "%PD%Debug\rps2_tests.dll" if "!DLL!"=="" set "DLL=%PD%Debug\rps2_tests.dll"
if exist "%PD%x64\Debug\rps2_tests.dll" if "!DLL!"=="" set "DLL=%PD%x64\Debug\rps2_tests.dll"
if exist "%PD%..\Release\rps2_tests.dll" if "!DLL!"=="" set "DLL=%PD%..\Release\rps2_tests.dll"
if exist "%PD%..\x64\Release\rps2_tests.dll" if "!DLL!"=="" set "DLL=%PD%..\x64\Release\rps2_tests.dll"

if "!DLL!"=="" (
  echo [RunVstestTests] rps2_tests.dll not found. Build the rps2_tests project first.
  exit /b 1
)

set "VSW=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSW%" (
  echo [RunVstestTests] vswhere not found. Install Visual Studio.
  exit /b 1
)
for /f "usebackq delims=" %%i in (`"%VSW%" -latest -products * -property installationPath`) do set "VSDIR=%%i"
if "!VSDIR!"=="" (
  echo [RunVstestTests] No Visual Studio installation found.
  exit /b 1
)
set "VSTEST=!VSDIR!\Common7\IDE\Extensions\TestPlatform\vstest.console.exe"
if not exist "!VSTEST!" (
  set "VSTEST=!VSDIR!\Common7\IDE\CommonExtensions\Microsoft\TestWindow\vstest.console.exe"
)
if not exist "!VSTEST!" (
  echo [RunVstestTests] vstest.console.exe not found.
  echo Looked: !VSDIR!\Common7\IDE\...
  exit /b 1
)

set "PLAT=/Platform:x86"
echo !DLL! | findstr /i "\\x64\\" >nul && set "PLAT=/Platform:x64"

echo.
echo [RunVstestTests] VSTest: !VSTEST!
echo [RunVstestTests]  DLL: !DLL! !PLAT!
echo.
"!VSTEST!" "!DLL!" !PLAT! /Logger:console;verbosity=normal
set "EX=!ERRORLEVEL!"
endlocal & exit /b %EX%
