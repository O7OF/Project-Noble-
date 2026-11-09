@echo off
setlocal
title Noble - C12-L 32-player smoke test (3 runs)
cd /d "%~dp0"
rem C12-L smoke test: one window, solo Sword Base Slayer, three runs one after another, separate logs.
rem Needs SETTINGS > ENGINE LIMITS = Extended (any Extended profile); -slot-patch is ignored on Stock.
rem Each run quits by itself after 120 s. Do not touch the keyboard/mouse during a run.

set "EXE=build\bin\Release\noble-dev.exe"
set "ARGS=-map 20_sword_slayer -mode multiplayer -quit-after 120"
if not exist "%EXE%" (
    echo %EXE% not found. Build it with build-dev.bat first.
    pause
    exit /b 1
)

rem Make sure no slot settings leak in from the environment.
set "NOBLE_SLOT_PATCH="
set "NOBLE_SLOT_PLAYERS="
set "NOBLE_SLOT_PLAYERS_UNSAFE="

echo.
echo [1/3] No slot options              -^> slot-smoke-1-none.log
start "" /wait "%EXE%" %ARGS% -log slot-smoke-1-none.log

echo [2/3] -slot-patch -slot-players 32 -^> slot-smoke-2-gated.log  (must refuse and stay 16)
start "" /wait "%EXE%" %ARGS% -slot-patch -slot-players 32 -log slot-smoke-2-gated.log

echo [3/3] Same + NOBLE_SLOT_PLAYERS_UNSAFE=1 -^> slot-smoke-3-unsafe.log
set "NOBLE_SLOT_PLAYERS_UNSAFE=1"
start "" /wait "%EXE%" %ARGS% -slot-patch -slot-players 32 -log slot-smoke-3-unsafe.log
set "NOBLE_SLOT_PLAYERS_UNSAFE="

echo.
echo Done. What to look for:
echo   1: no "SlotPatch:" install lines, match runs 120 s, no FATAL / STACK OVERRUN.
echo   2: "SlotPatch: -slot-players 32 refused" and "16-player mode"; pools stay 16; no crash.
echo   3: "NOBLE_SLOT_PLAYERS_UNSAFE=1 - enabling", every patch installed (no "failed"/"mismatch"),
echo      PoolResize players / fire team / simulation view at 32, distributed view line; no crash.
echo.
findstr /i /c:"SlotPatch" /c:"PoolResize" /c:"FATAL" /c:"STACK OVERRUN" slot-smoke-1-none.log slot-smoke-2-gated.log slot-smoke-3-unsafe.log
echo.
pause
