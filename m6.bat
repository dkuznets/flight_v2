@echo off

set DD=%DATE:.=%
set TT=%TIME:~0,-6%
set TT=%TT::=%
set TT=%TT: =0%
set DT=%DD%%TT%

set DD=%DD:~-4%%DD:~2,-4%%DD:~0,-6%

Setlocal EnableDelayedExpansion
set newFN=%DT%

Setlocal DisableDelayedExpansion
echo --------------------------------------------------------------------------------
echo Converting file "firmware.afx" to "%newFN%.bin"
echo --------------------------------------------------------------------------------
