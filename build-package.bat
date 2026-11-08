@echo off
setlocal
cd /d "%~dp0"
title Project Noble - Packager

powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0build-package.ps1"
set "PKG_ERR=%ERRORLEVEL%"
if not "%PKG_ERR%"=="0" (
    echo ===================================================
    echo  Packaging failed with error code %PKG_ERR%.
    echo ===================================================
)
if "%~1"=="" pause
exit /b %PKG_ERR%
