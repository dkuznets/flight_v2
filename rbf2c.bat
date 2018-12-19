@echo off

echo --------------------------------------------------------------------------------
echo Converting file "plis1_fw.rbf" to "plis1_fw.c"...
echo.
"..\Utils\rbf2c\rbf2c.exe" "..\PLIS_fw\plis1_fw.rbf" "PLIS\plis1_fw.c" plis1_data 65535

echo --------------------------------------------------------------------------------
echo Converting file "plis2_fw.rbf" to "plis2_fw.c"...
echo.
"..\Utils\rbf2c\rbf2c.exe" "..\PLIS_fw\plis2_fw.rbf" "PLIS\plis2_fw.c" plis2_data 65535

echo --------------------------------------------------------------------------------
