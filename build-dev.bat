@echo off
setlocal enabledelayedexpansion
cd /d "%~dp0"
title Halo: Reach Community Edition - Developer Builder

echo ===================================================
echo  Building Noble - DEVELOPER build (devtools enabled)
echo ===================================================

set "BUILD_DIR=build-dev"

if /i "%~1"=="clean" (
    echo [*] Removing %BUILD_DIR%...
    if exist "%BUILD_DIR%" rd /s /q "%BUILD_DIR%"
    goto :end
)

where cmake >nul 2>nul
if %ERRORLEVEL% neq 0 set "PATH=C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;%PATH%"
where cmake >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo [ERROR] CMake not found. Install Visual Studio C++ tools or CMake.
    goto :failed
)

echo [1/2] Configuring %BUILD_DIR% with NOBLE_DEV_BUILD=ON...
cmake -S . -B "%BUILD_DIR%" -A x64 -DNOBLE_DEV_BUILD=ON
if %ERRORLEVEL% neq 0 (
    echo [!] Configure failed; clearing the dev cache and retrying...
    if exist "%BUILD_DIR%\CMakeCache.txt" del /f /q "%BUILD_DIR%\CMakeCache.txt"
    if exist "%BUILD_DIR%\CMakeFiles" rd /s /q "%BUILD_DIR%\CMakeFiles"
    cmake -S . -B "%BUILD_DIR%" -A x64 -DNOBLE_DEV_BUILD=ON
    if !ERRORLEVEL! neq 0 goto :failed
)

echo [2/2] Building Developer Release...
cmake --build "%BUILD_DIR%" --config Release
if %ERRORLEVEL% neq 0 goto :failed

echo ===================================================
echo  Developer build succeeded!
echo  Executable: %BUILD_DIR%\bin\Release\noble-client.exe
echo  Dev menu and devtools are enabled by default in this build.
echo ===================================================
goto :end

:failed
echo ===================================================
echo  Developer build failed.
echo ===================================================

:end
if "%~1"=="" pause
