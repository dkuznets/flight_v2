@echo off
if "%1" EQU "" goto error
if "%2" EQU "" goto error

SetLocal EnableDelayedExpansion

set DD=%DATE:.=%
set year2=%DD:~-4%
set month2=%DD:~2,-4%
set day2=%DD:~0,-6%

set /a c=0
if "%2" EQU "L" (
	for /f "UseBackQ Delims=" %%A IN ("ver.txt") do (
		set /a c+=1
		if !c!==3 set year=%%A
		if !c!==4 set month=%%A
		if !c!==5 set day=%%A
		if !c!==6 set build=%%A
		set file="ver.txt"
		set hfile="ver.h"
	)
)

if "%2" EQU "R" (	
	for /f "UseBackQ Delims=" %%A IN ("ver.txt") do (
		set /a c+=1
		if !c!==3 set year=%%A
		if !c!==4 set month=%%A
		if !c!==5 set day=%%A
		if !c!==6 set build=%%A
		set file="ver.txt"
		set hfile="ver.h"
	)
)

if "%2" EQU "U" (	
	for /f "UseBackQ Delims=" %%A IN ("ver.txt") do (
		set /a c+=1
		if !c!==3 set year=%%A
		if !c!==4 set month=%%A
		if !c!==5 set day=%%A
		if !c!==6 set build=%%A
		set file="ver.txt"
		set hfile="ver.h"
	)
)

set /a year=10000%year% %% 10000
set /a month=10000%month% %% 10000
set /a day=10000%day% %% 10000
set /a build=10000%build% %% 10000

set /a year2=10000%year2% %% 10000
set /a month2=10000%month2% %% 10000
set /a day2=10000%day2% %% 10000

rem echo %1
rem echo %2
rem echo %year%
rem echo %month%
rem echo %day%
rem echo %build%

rem echo %DD1%
rem echo %DD2%
rem echo %DD3%

if %year% EQU %year2% (
	if %month% EQU %month2% (
		if %day% EQU %day2% ( 
			set /a build+=1 
		) else ( 
			set /a build=1
		)
	) else (
		set /a build=1
	)
) else (
	set /a build=1
)
rem echo.%build%

echo #define ITYPE "%1" > %hfile%
echo #define IWING "%2" >> %hfile%
echo #define IYEAR %year2% >> %hfile%
echo #define IMON %month2% >> %hfile%
echo #define IDAY %day2% >> %hfile%
echo #define IVER %build% >> %hfile%

echo "%1" > %file%
echo "%2" >> %file%
echo %year2% >> %file%
echo %month2% >> %file%
echo %day2% >> %file%
echo %build% >> %file%

echo New version %year2%%month2%%day2%_%build%
Setlocal DisableDelayedExpansion

goto exit
:error
Setlocal DisableDelayedExpansion
echo --------------------------------------------------------------------------------
echo.
echo ERROR!!! No file name!!!
echo.
echo --------------------------------------------------------------------------------
goto exit
:exit

