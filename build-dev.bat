@echo off
setlocal enabledelayedexpansion
cd /d "%~dp0"
title Project Noble - Developer Builder

echo ===================================================
echo  Building Noble - DEVELOPER (noble-dev.exe)
echo ===================================================

where git >nul 2>nul
if %ERRORLEVEL% neq 0 (
    if exist "C:\Program Files\Git\cmd\git.exe" set "PATH=C:\Program Files\Git\cmd;!PATH!"
)

set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
where cmake >nul 2>nul
if %ERRORLEVEL% neq 0 (
    if exist "C:\Program Files\CMake\bin\cmake.exe" (
        set "PATH=C:\Program Files\CMake\bin;!PATH!"
    ) else if exist "!VSWHERE!" (
        for /f "usebackq tokens=*" %%i in (`"!VSWHERE!" -latest -property installationPath`) do (
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
        ) else if exist "%LOCALAPPDATA%\Microsoft\WinGet\Links\cmake.exe" (
            set "PATH=%LOCALAPPDATA%\Microsoft\WinGet\Links;!PATH!"
        )
    )
)
where cmake >nul 2>nul
if %ERRORLEVEL% neq 0 (
    echo [ERROR] CMake not found.
    echo ===================================================
    echo  Developer build failed with error code 1.
    echo ===================================================
    if "%~1"=="" pause
    exit /b 1
)

set "HAS_VS_CPP=0"
if exist "!VSWHERE!" (
    for /f "usebackq tokens=*" %%i in (`"!VSWHERE!" -latest -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
        if not "%%i"=="" set "HAS_VS_CPP=1"
    )
)

set "USE_NINJA_MSVC=0"
if "%HAS_VS_CPP%"=="0" (
    if exist "%LOCALAPPDATA%\Programs\MSVC\VC\Tools\MSVC" (
        for /f "delims=" %%v in ('dir /b /ad "%LOCALAPPDATA%\Programs\MSVC\VC\Tools\MSVC"') do set "MSVC_VER=%%v"
        set "MSVC_DIR=%LOCALAPPDATA%\Programs\MSVC\VC\Tools\MSVC\!MSVC_VER!"
        set "SDK_DIR=C:\Program Files\Microsoft Visual Studio\2022\Community\SDK\ScopeCppSDK\vc15\SDK"
        if not exist "!SDK_DIR!\include\um\Windows.h" (
            if exist "C:\Program Files\Microsoft Visual Studio\18\Community\SDK\ScopeCppSDK\vc15\SDK\include\um\Windows.h" (
                set "SDK_DIR=C:\Program Files\Microsoft Visual Studio\18\Community\SDK\ScopeCppSDK\vc15\SDK"
            )
        )
        set "NINJA_DIR=%LOCALAPPDATA%\Microsoft\WinGet\Packages\Ninja-build.Ninja_Microsoft.Winget.Source_8wekyb3d8bbwe"
        if not exist "!NINJA_DIR!\ninja.exe" set "NINJA_DIR=%LOCALAPPDATA%\Microsoft\WinGet\Links"
        if exist "!MSVC_DIR!\bin\Hostx64\x64\cl.exe" (
            set "PATH=!MSVC_DIR!\bin\Hostx64\x64;!SDK_DIR!\bin;!NINJA_DIR!;!PATH!"
            set "INCLUDE=!MSVC_DIR!\include;!SDK_DIR!\include\ucrt;!SDK_DIR!\include\um;!SDK_DIR!\include\shared"
            set "LIB=!MSVC_DIR!\lib\x64;!SDK_DIR!\lib"
            set "USE_NINJA_MSVC=1"
        )
    )
)

set "DO_CONFIG=0"
if not exist "build\CMakeCache.txt" (
    set "DO_CONFIG=1"
) else (
    if "%USE_NINJA_MSVC%"=="1" (
        findstr /C:"CMAKE_GENERATOR:INTERNAL=Ninja" "build\CMakeCache.txt" >nul 2>nul
        if errorlevel 1 (
            set "DO_CONFIG=1"
            if exist "build\CMakeCache.txt" del /f /q "build\CMakeCache.txt" >nul 2>nul
            if exist "build\CMakeFiles" rd /s /q "build\CMakeFiles" >nul 2>nul
        )
    ) else (
        findstr /C:"CMAKE_GENERATOR:INTERNAL=Visual Studio" "build\CMakeCache.txt" >nul 2>nul
        if errorlevel 1 (
            set "DO_CONFIG=1"
            if exist "build\CMakeCache.txt" del /f /q "build\CMakeCache.txt" >nul 2>nul
            if exist "build\CMakeFiles" rd /s /q "build\CMakeFiles" >nul 2>nul
        )
    )
)

if "%DO_CONFIG%"=="1" (
    echo [1/2] Configuring build...
    if "%USE_NINJA_MSVC%"=="1" (
        cmake -S . -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXE_LINKER_FLAGS="/machine:x64 /MANIFEST:NO" -DCMAKE_SHARED_LINKER_FLAGS="/machine:x64 /MANIFEST:NO"
    ) else (
        cmake -S . -B build -A x64
    )
    if !ERRORLEVEL! neq 0 (
        set "BUILD_ERR=!ERRORLEVEL!"
        echo ===================================================
        echo  Developer build failed with error code !BUILD_ERR!.
        echo ===================================================
        if "%~1"=="" pause
        exit /b !BUILD_ERR!
    )
) else (
    echo [1/2] CMake cache verified for x64.
)

echo [2/2] Building noble-dev...
taskkill /F /IM noble-dev.exe >nul 2>nul
cmake --build build --config Release --target noble-dev
if %ERRORLEVEL% neq 0 (
    set "BUILD_ERR=%ERRORLEVEL%"
    echo ===================================================
    echo  Developer build failed with error code !BUILD_ERR!.
    echo ===================================================
    if "%~1"=="" pause
    exit /b !BUILD_ERR!
)

if exist "data" (
    robocopy "data" "build\bin\Release\data" /E /XO /XD userdata autosave mods cache sandbox /NFL /NDL /NJH /NJS /NP >nul 2>nul
)
if exist "assets" (
    robocopy "assets" "build\bin\Release\data" /E /XO /NFL /NDL /NJH /NJS /NP >nul 2>nul
)
if exist "tools\vgmstream" (
    robocopy "tools\vgmstream" "build\bin\Release\tools\vgmstream" /E /XO /NFL /NDL /NJH /NJS /NP >nul 2>nul
)

echo ===================================================
echo  Developer build succeeded.
echo  Executable: build\bin\Release\noble-dev.exe
echo ===================================================
if "%~1"=="" pause
exit /b 0
