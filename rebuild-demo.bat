@echo off
setlocal

set "ROOT_DIR=%~dp0"
cd /d "%ROOT_DIR%"

set "PLATFORM=Win32"
set "CONFIG=Release"
set "BUILD_DIR=reverse\work\reconstruction\build-win32"
set "TARGET=cornball_demo_replay"

if /I "%~1"=="x64" (
    set "PLATFORM=x64"
    set "BUILD_DIR=reverse\work\reconstruction\build"
)

if /I "%~1"=="win32" (
    set "PLATFORM=Win32"
    set "BUILD_DIR=reverse\work\reconstruction\build-win32"
)

if /I "%~2"=="debug" (
    set "CONFIG=Debug"
)

if /I "%~2"=="release" (
    set "CONFIG=Release"
)

echo Rebuilding %TARGET%...
echo   platform: %PLATFORM%
echo   config:   %CONFIG%
echo   build:    %BUILD_DIR%
echo.

cmake -S reverse/work/reconstruction -B "%BUILD_DIR%" -G "Visual Studio 17 2022" -A %PLATFORM%
if errorlevel 1 goto :error

cmake --build "%BUILD_DIR%" --config %CONFIG% --target %TARGET%
if errorlevel 1 goto :error

echo.
echo Build completed:
echo   %BUILD_DIR%\%CONFIG%\%TARGET%.exe
exit /b 0

:error
echo.
echo Build failed.
exit /b 1
