@echo off
setlocal
echo ===================================================
echo  Project Noble - Publish Release
echo ===================================================

powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0tools\publish-release.ps1" %*
set "PUB_ERR=%ERRORLEVEL%"

if %PUB_ERR% neq 0 (
    echo ===================================================
    echo  Publish failed with error code %PUB_ERR%.
    echo ===================================================
    if "%~1"=="" pause
    exit /b %PUB_ERR%
)

echo ===================================================
echo  Release published successfully.
echo ===================================================
if "%~1"=="" pause
exit /b 0
