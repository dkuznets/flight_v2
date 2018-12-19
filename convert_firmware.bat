@echo off
echo --------------------------------------------------------------------------------
if "%1" EQU "" (
	echo.
	echo One parameter needed!
	goto error
)
echo Converting file "plis1_fw.rbf" to "plis1_fw.c"...
echo.
"..\..\utils\rbf2c.exe" "%1\plis1_fw.rbf" "..\RTX\PLIS\plis1_fw.c" plis1_data 65535
if NOT ERRORLEVEL 0 goto error
echo --------------------------------------------------------------------------------
echo Converting file "plis2_fw.rbf" to "plis2_fw.c"...
echo.
"..\..\utils\rbf2c.exe" "%1\plis2_fw.rbf" "..\RTX\PLIS\plis2_fw.c" plis2_data 65535
if NOT ERRORLEVEL 0 goto error
echo --------------------------------------------------------------------------------
echo Converting files Done!
goto exit
:error
echo Error converting file!
echo.
echo --------------------------------------------------------------------------------
:exit