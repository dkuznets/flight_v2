@echo off

set KEIL_UTILITY_DEFAULT="C:\Keil\ARM\BIN40\fromelf.exe"
set KEIL_UTILITY_ALTERNATIVE="C:\KeilARM\ARM\BIN40\fromelf.exe"
set KEIL_UTILITY=%KEIL_UTILITY_DEFAULT%

if exist %KEIL_UTILITY_ALTERNATIVE% set KEIL_UTILITY=%KEIL_UTILITY_ALTERNATIVE%

set INPUT_FILE="%cd%\Output\firmware_left_wing.axf"
set OUTPUT_FILE="%cd%\firmware_solo2_flight_left_wing.bin"

echo -------------------------------------------------------------------------------
echo Converting ".AXF" file to ".BIN"...

if not exist %KEIL_UTILITY% (
    echo ERROR - file %KEIL_UTILITY% does not exist
    echo Input file was not converted!
    goto exit
)

echo Input file %INPUT_FILE%
echo Output file %OUTPUT_FILE%

%KEIL_UTILITY% --output %OUTPUT_FILE% --bin %INPUT_FILE%
echo Done

:exit

echo -------------------------------------------------------------------------------
