@echo off
setlocal enabledelayedexpansion
cd /d "%~dp0"
title Halo: Reach Community Edition - Builder

echo ===================================================
echo  Building Halo: Reach Community Edition - Project Noble
echo ===================================================

if /i "%~1"=="clean" (
    echo [*] Cleaning build directory...
    if exist "build" (
        rd /s /q "build"
        echo [*] Build directory removed.
    ) else (
        echo [*] No build directory found.
    )
    goto :end
)

REM Locate CMake if not present in system PATH
where cmake >nul 2>nul
if %ERRORLEVEL% neq 0 (
    if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" (
        for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -property installationPath`) do (
            if exist "%%i\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" (
                set "PATH=%%i\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;!PATH!"
            )
        )
    )
    where cmake >nul 2>nul
    if !ERRORLEVEL! neq 0 (
        if exist "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" (
            set "PATH=C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;!PATH!"
        ) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" (
            set "PATH=C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;!PATH!"
        )
    )
)

where cmake >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo [ERROR] CMake could not be found. Please ensure CMake or Visual Studio C++ tools are installed.
    set BUILD_ERR=1
    goto :build_failed
)

echo [1/3] Syncing latest commits from git origin/main...
git fetch origin main 2>nul
if %ERRORLEVEL% equ 0 (
    git pull origin main 2>nul
) else (
    echo [i] Note: Git remote sync skipped - offline or no remote access. Continuing local build...
)

set "DO_CONFIG=0"
if /i "%~1"=="rebuild" (
    echo [*] Rebuild requested. Clearing CMake cache and subbuilds...
    set "DO_CONFIG=1"
    call :clean_cache
)

if not exist "build\CMakeCache.txt" (
    set "DO_CONFIG=1"
) else (
    REM Validate that the existing cache was generated with Visual Studio multi-config generator
    findstr /C:"CMAKE_GENERATOR:INTERNAL=Visual Studio" "build\CMakeCache.txt" >nul 2>nul
    if errorlevel 1 (
        echo [!] Generator mismatch detected in build\CMakeCache.txt - expected Visual Studio x64.
        echo [!] Clearing mismatched cache and subbuild states to avoid conflicts...
        set "DO_CONFIG=1"
        call :clean_cache
    )
)

if "%DO_CONFIG%"=="1" (
    echo [2/3] Configuring CMake project for Visual Studio x64...
    cmake -S . -B build -A x64
    if !ERRORLEVEL! neq 0 (
        echo [!] CMake configuration failed. Attempting full cache reset and retry...
        call :clean_cache
        cmake -S . -B build -A x64
        if !ERRORLEVEL! neq 0 (
            echo [ERROR] CMake configuration failed permanently.
            set BUILD_ERR=!ERRORLEVEL!
            goto :build_failed
        )
    )
) else (
    echo [2/3] CMake cache verified for Visual Studio x64.
)

echo [3/3] Building Release configuration...
cmake --build build --config Release
if %ERRORLEVEL% neq 0 (
    set BUILD_ERR=%ERRORLEVEL%
    goto :build_failed
)

echo ===================================================
echo  Build succeeded!
echo  Executable: build\bin\Release\noble-client.exe
echo ===================================================
goto :end

:clean_cache
if exist "build\CMakeCache.txt" del /f /q "build\CMakeCache.txt" >nul 2>nul
if exist "build\CMakeFiles" rd /s /q "build\CMakeFiles" >nul 2>nul
for /d %%D in ("build\_deps\*-subbuild") do (
    if exist "%%D" rd /s /q "%%D" >nul 2>nul
)
exit /b 0

:build_failed
echo ===================================================
echo  Build failed with error code %BUILD_ERR%.
echo ===================================================

:end
if "%~1"=="" pause
