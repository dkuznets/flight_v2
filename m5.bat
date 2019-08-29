@echo off
if "%1" EQU "" goto error
rem set FF=firmware_select_long_time

set DD=%DATE:.=%
set TT=%TIME:~0,-6%
set TT=%TT::=%
set DT=%DD%_%TT%

set DD=%DD:~-4%%DD:~2,-4%%DD:~0,-6%

Setlocal EnableDelayedExpansion
set UU=0
set NUM=0
set TPP=

for %%i in (..\..\FW\%1_%DD%*.bin) do (
	set TPP= %%~ni
	set NUM=!TPP:~-2!
	if !NUM! GEQ !UU! (set UU=!NUM!)
)
rem echo Last num %UU%
if %UU% LSS 10 (set UU=!UU:~1!)
rem echo Last num %UU%

if "%UU%" EQU "99" (goto err2)

set /A UU=%UU%+1
if %UU% LSS 10 (set UU=0!UU!)
rem set newFN=%1_%DD%_%UU%
set newFN=%1

Setlocal DisableDelayedExpansion
echo --------------------------------------------------------------------------------
echo Converting file "firmware.afx" to "%newFN%.bin"
echo.
d:\Keil_v473\ARM\ARMCC\bin\fromelf.exe --output "..\..\FW\%newFN%.bin" --bin "Output\firmware.axf"
echo Converting file Done!
if "%2" EQU "nocs" goto nocs
echo --------------------------------------------------------------------------------
echo Converting file "%newFN%.bin"to "%1.cs"...
echo.
"..\..\utils\bin2cs.exe" "..\..\FW\%newFN%.bin" %1 "d:\work\OLO\ex\OLO_CAN"
echo.
echo Converting file Done!
echo --------------------------------------------------------------------------------
:nocs
if NOT ERRORLEVEL 0 goto err3
goto exit

:error
Setlocal DisableDelayedExpansion
echo --------------------------------------------------------------------------------
echo.
echo ERROR!!! No file name!!!
echo.
echo --------------------------------------------------------------------------------
goto exit
:err2
Setlocal DisableDelayedExpansion
echo --------------------------------------------------------------------------------
echo.
echo ERROR !!! Too many files!!!
echo.
echo --------------------------------------------------------------------------------
goto exit
:err3
Setlocal DisableDelayedExpansion
echo --------------------------------------------------------------------------------
echo.
echo ERROR !!! Converting to CS failed!!!
echo.
echo --------------------------------------------------------------------------------
goto exit
:exit
