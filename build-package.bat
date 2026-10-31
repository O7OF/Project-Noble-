@echo off
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0build-package.ps1"
if "%~1"=="" pause
