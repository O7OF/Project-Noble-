@echo off
setlocal enabledelayedexpansion
cd /d "%~dp0"
title Halo: Reach Community Edition - Developer Builder

echo ===================================================
echo  Building Noble - DEVELOPER (noble-dev.exe)
echo ===================================================

where cmake >nul 2>nul
if %ERRORLEVEL% neq 0 set "PATH=C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;%PATH%"
where cmake >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo [ERROR] CMake not found.
    goto :failed
)

if not exist "build\CMakeCache.txt" (
    echo [1/2] Configuring build...
    cmake -S . -B build -A x64
    if !ERRORLEVEL! neq 0 goto :failed
)

echo [2/2] Building noble-dev...
cmake --build build --config Release --target noble-dev
if %ERRORLEVEL% neq 0 goto :failed

echo ===================================================
echo  Developer build succeeded!
echo  Executable: build\bin\Release\noble-dev.exe
echo ===================================================
goto :end

:failed
echo ===================================================
echo  Developer build failed.
echo ===================================================

:end
if "%~1"=="" pause
