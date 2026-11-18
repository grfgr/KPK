@echo off
rem Converts every PNG texture in P:\KPK\data to PAA with ImageToPAA from DayZ Tools.
rem Usage: double-click. If DayZ Tools live elsewhere, drag ImageToPAA.exe onto this file.
chcp 65001 >nul
set "TOOL=C:\Program Files (x86)\Steam\steamapps\common\DayZ Tools\Bin\ImageToPAA\ImageToPAA.exe"
if not "%~1"=="" set "TOOL=%~1"
set "DATA=P:\KPK\data"
if not exist "%TOOL%" (
  echo ImageToPAA.exe not found: "%TOOL%"
  echo Drag ImageToPAA.exe from your DayZ Tools folder onto this .bat file.
  pause
  exit /b 1
)
for %%f in ("%DATA%\*.png") do (
  echo %%~nxf  -^>  %%~nf.paa
  "%TOOL%" "%%f" "%DATA%\%%~nf.paa"
)
echo.
echo Done.
pause
