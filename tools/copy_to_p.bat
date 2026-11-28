@echo off
setlocal
rem Copies the mod folder KPK (config.cpp, scripts, gui, data) to P:\KPK for Addon Builder.
rem Run it from the repository folder (double click), not from drive P:.
cd /d "%~dp0.."
if not exist P:\ goto nop
if not exist "KPK\config.cpp" goto norepo
xcopy "KPK" "P:\KPK\" /E /I /Y /Q
echo.
echo DONE: P:\KPK. Next: Addon Builder, source P:\KPK, prefix KPK, Pack.
if not "%~1"=="nopause" pause
exit /b 0

:nop
echo ERROR: drive P: is not mounted. Run DayZ Tools - Workdrive - Mount.
if not "%~1"=="nopause" pause
exit /b 1

:norepo
echo ERROR: KPK\config.cpp not found in %CD%. Run this file from the repository folder.
if not "%~1"=="nopause" pause
exit /b 1
