@echo off
title Project Noble (Closed Source)
cd /d "%~dp0"

echo ===================================================
echo  Launching Project Noble...
echo ===================================================

if exist "build\bin\Release\noble-client.exe" (
    start "" "build\bin\Release\noble-client.exe"
) else (
    echo Error: noble-client.exe not found in build\bin\Release\
    echo Please build the project first or run cmake build.
    pause
)
