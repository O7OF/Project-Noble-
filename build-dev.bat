@echo off
setlocal enabledelayedexpansion
cd /d "%~dp0"
title Halo: Reach Community Edition - Developer Builder

echo ===================================================
echo  Building Noble - DEVELOPER (noble-dev.exe)
echo ===================================================

where cmake >nul 2>nul
if %ERRORLEVEL% neq 0 (
    if exist "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" (
        set "PATH=C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;%PATH%"
    ) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" (
        set "PATH=C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;%PATH%"
    )
)
where cmake >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo [ERROR] CMake not found.
    goto :failed
)

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

if not exist "build\CMakeCache.txt" (
    echo [1/2] Configuring build...
    if "%USE_NINJA_MSVC%"=="1" (
        cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
    ) else (
        cmake -S . -B build -A x64
    )
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
