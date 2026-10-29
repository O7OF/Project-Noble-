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

REM Check whether a Visual Studio instance with C++ build tools is installed
set "HAS_VS_CPP=0"
if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" (
    for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
        if not "%%i"=="" set "HAS_VS_CPP=1"
    )
)

set "USE_NINJA_MSVC=0"
if "%HAS_VS_CPP%"=="0" (
    if exist "%LOCALAPPDATA%\Programs\MSVC\VC\Tools\MSVC" (
        for /f "delims=" %%v in ('dir /b /ad "%LOCALAPPDATA%\Programs\MSVC\VC\Tools\MSVC"') do set "MSVC_VER=%%v"
        set "MSVC_DIR=%LOCALAPPDATA%\Programs\MSVC\VC\Tools\MSVC\!MSVC_VER!"
        set "SDK_DIR=C:\Program Files\Microsoft Visual Studio\2022\Community\SDK\ScopeCppSDK\vc15\SDK"
        set "NINJA_DIR=%LOCALAPPDATA%\Microsoft\WinGet\Packages\Ninja-build.Ninja_Microsoft.Winget.Source_8wekyb3d8bbwe"
        if exist "!MSVC_DIR!\bin\Hostx64\x64\cl.exe" (
            set "PATH=!MSVC_DIR!\bin\Hostx64\x64;!SDK_DIR!\bin;!NINJA_DIR!;!PATH!"
            set "INCLUDE=!MSVC_DIR!\include;!SDK_DIR!\include\ucrt;!SDK_DIR!\include\um;!SDK_DIR!\include\shared"
            set "LIB=!MSVC_DIR!\lib\x64;!SDK_DIR!\lib"
            set "USE_NINJA_MSVC=1"
        )
    )
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
    if "%USE_NINJA_MSVC%"=="1" (
        findstr /C:"CMAKE_GENERATOR:INTERNAL=Ninja" "build\CMakeCache.txt" >nul 2>nul
        if errorlevel 1 (
            set "DO_CONFIG=1"
            call :clean_cache
        )
    ) else (
        findstr /C:"CMAKE_GENERATOR:INTERNAL=Visual Studio" "build\CMakeCache.txt" >nul 2>nul
        if errorlevel 1 (
            echo [!] Generator mismatch detected in build\CMakeCache.txt - expected Visual Studio x64.
            echo [!] Clearing mismatched cache and subbuild states to avoid conflicts...
            set "DO_CONFIG=1"
            call :clean_cache
        )
    )
)

if "%DO_CONFIG%"=="1" (
    if "%USE_NINJA_MSVC%"=="1" (
        echo [2/3] Configuring CMake project for Ninja + MSVC x64...
        cmake -S . -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXE_LINKER_FLAGS="/machine:x64 /MANIFEST:NO" -DCMAKE_SHARED_LINKER_FLAGS="/machine:x64 /MANIFEST:NO"
    ) else (
        echo [2/3] Configuring CMake project for Visual Studio x64...
        cmake -S . -B build -A x64
    )
    if !ERRORLEVEL! neq 0 (
        echo [!] CMake configuration failed. Attempting full cache reset and retry...
        call :clean_cache
        if "%USE_NINJA_MSVC%"=="1" (
            cmake -S . -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXE_LINKER_FLAGS="/machine:x64 /MANIFEST:NO" -DCMAKE_SHARED_LINKER_FLAGS="/machine:x64 /MANIFEST:NO"
        ) else (
            cmake -S . -B build -A x64
        )
        if !ERRORLEVEL! neq 0 (
            echo [ERROR] CMake configuration failed permanently.
            set BUILD_ERR=!ERRORLEVEL!
            goto :build_failed
        )
    )
) else (
    echo [2/3] CMake cache verified for x64.
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
