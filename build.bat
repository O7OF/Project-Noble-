@echo off
setlocal
cd /d "%~dp0"
title Halo: Reach Community Edition - Builder

echo ===================================================
echo  Building Halo: Reach Community Edition - Project Noble
echo ===================================================

where cmake >nul 2>nul
if %ERRORLEVEL% neq 0 (
    if exist "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" (
        set "PATH=C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;%PATH%"
    ) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" (
        set "PATH=C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;%PATH%"
    )
)

echo [1/3] Syncing latest commits from git origin/main...
git fetch origin main
git pull origin main

if not exist "build\CMakeCache.txt" (
    echo [2/3] Configuring CMake project x64...
    cmake -S . -B build -A x64
)

echo [3/3] Building Release configuration...
cmake --build build --config Release

if %ERRORLEVEL% equ 0 (
    echo ===================================================
    echo  Build succeeded!
    echo  Executable: build\bin\Release\noble-client.exe
    echo ===================================================
) else (
    echo ===================================================
    echo  Build failed with error code %ERRORLEVEL%.
    echo ===================================================
)

if "%~1"=="" pause
