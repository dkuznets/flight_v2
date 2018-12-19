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
echo Last num %UU%
if %UU% LSS 10 (set UU=!UU:~1!)
echo Last num %UU%

if "%UU%" EQU "99" (goto err2)

set /A UU=%UU%+1
if %UU% LSS 10 (set UU=0!UU!)
set newFN=%1_%DD%_%UU%

Setlocal DisableDelayedExpansion
echo --------------------------------------------------------------------------------
echo Converting file "firmware.afx" to "%newFN%.bin"
echo.
"d:\Keil_v473\ARM\ARMCC\bin\fromelf.exe" --output "..\..\FW\%newFN%.bin" --bin "Output\firmware.axf"
echo Converting file Done!
echo --------------------------------------------------------------------------------
echo Converting file "%newFN%.bin"to "%1.cs"...
echo.
"..\..\utils\bin2cs.exe" "..\..\FW\%newFN%.bin" "%1" "d:\work\OLO\ex\OLO_CAN"
echo.
echo Converting file Done!
echo --------------------------------------------------------------------------------
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


echo @echo off > "..\..\FW\%1.bat"
echo d:\work\OLO\ex\Release\LPCRC.exe d:\work\OLO\SOLO2_FW\FW\%newFN%.bin >> "..\..\FW\%1.bat"
echo "c:\Program Files (x86)\SEGGER\JLink_V600a\JFlash.exe" -openprjd:\work\OLO\SOLO2_FW\FW\solo2.jflash -opend:\work\OLO\SOLO2_FW\FW\%newFN%.bin,0x0 -erasechip -programverify -startapp -exit  >> "..\..\FW\%1.bat"
echo IF ERRORLEVEL 1 goto ERROR >> "..\..\FW\%1.bat"
echo goto END >> "..\..\FW\%1.bat"
echo :ERROR >> "..\..\FW\%1.bat"
echo color C >> "..\..\FW\%1.bat"
echo ECHO J-Flash ARM: Error! >> "..\..\FW\%1.bat"
echo color D >> "..\..\FW\%1.bat"
echo ECHO J-Flash ARM: Error! >> "..\..\FW\%1.bat"
echo color >> "..\..\FW\%1.bat"
echo pause >> "..\..\FW\%1.bat"
echo :END >> "..\..\FW\%1.bat"
