SET PATH=d:\Keil_v473\ARM\ARMCC\bin\;C:\Program Files (x86)\Common Files\Oracle\Java\javapath;C:\ProgramData\Oracle\Java\javapath;C:\PROGRA~2\Borland\CBUILD~1\Bin;C:\PROGRA~2\Borland\CBUILD~1\Projects\Bpl;C:\Program Files (x86)\Intel\iCLS Client\;C:\Program Files\Intel\iCLS Client\;C:\Windows\system32;C:\Windows;C:\Windows\System32\Wbem;C:\Windows\System32\WindowsPowerShell\v1.0\;C:\Program Files\Intel\Intel(R) Management Engine Components\DAL;C:\Program Files (x86)\Intel\Intel(R) Management Engine Components\DAL;C:\Program Files\Intel\Intel(R) Management Engine Components\IPT;C:\Program Files (x86)\Intel\Intel(R) Management Engine Components\IPT;C:\Program Files (x86)\Windows Kits\8.1\Windows Performance Toolkit\;C:\Program Files\Microsoft SQL Server\110\Tools\Binn\;C:\Program Files\010 Editor;C:\PROGRA~2\PICC;c:\altera\90\quartus\bin;C:\Program Files\TortoiseGit\bin;c:\Program Files (x86)\Java\jre1.8.0_121\bin;c:\Qt\5.9.1\msvc2013_64\bin;C:\Program Files (x86)\MATLAB\MATLAB Runtime\v90\runtime\win32;C:\Program Files (x86)\nodejs\;C:\Program Files (x86)\Atmel\sam-ba_2.16\drv\;C:\Program Files (x86)\Atmel\sam-ba_2.16;C:\Program Files (x86)\ATMEL Corporation\AT91-ISP v1.13\Library\;C:\Program Files (x86)\ATMEL Corporation\AT91-ISP v1.13\sam-ba 2.9;C:\Program Files (x86)\Atmel\sam-ba_2.15\drv\;C:\Program Files (x86)\Atmel\sam-ba_2.15;C:\Program Files (x86)\Atmel\sam-ba_2.12\drv\;C:\Program Files (x86)\Atmel\sam-ba_2.12;c:\Qt\5.9.1\msvc2013_64\bin;C:\Users\dkuznets\AppData\Roaming\npm
SET CPU_TYPE=LPC2294/01
SET CPU_VENDOR=NXP (founded by Philips)
SET UV2_TARGET=SOLO2 Right-wing
SET CPU_CLOCK=0x03938700
ver.bat FL R
"d:\Keil_v473\ARM\ARMCC\bin\ArmAsm" --Via ".\obj\startup._ia"
"d:\Keil_v473\ARM\ARMCC\bin\ArmCC" --Via ".\obj\rtx_config.__i"
"d:\Keil_v473\ARM\ARMCC\bin\ArmCC" --Via ".\obj\file_config.__i"
"d:\Keil_v473\ARM\ARMCC\bin\ArmCC" --Via ".\obj\main.__i"
"d:\Keil_v473\ARM\ARMCC\bin\ArmCC" --Via ".\obj\common.__i"
"d:\Keil_v473\ARM\ARMCC\bin\ArmCC" --Via ".\obj\upload.__i"
"d:\Keil_v473\ARM\ARMCC\bin\ArmCC" --Via ".\obj\compress.__i"
"d:\Keil_v473\ARM\ARMCC\bin\ArmCC" --Via ".\obj\plis1_fw.__i"
"d:\Keil_v473\ARM\ARMCC\bin\ArmCC" --Via ".\obj\plis2_fw.__i"
"d:\Keil_v473\ARM\ARMCC\bin\ArmCC" --Via ".\obj\can_lpc229x_custom.__i"
"d:\Keil_v473\ARM\ARMCC\bin\ArmCC" --Via ".\obj\rtx_can.__i"
"d:\Keil_v473\ARM\ARMCC\bin\ArmCC" --Via ".\obj\rtx_can_ex.__i"
"d:\Keil_v473\ARM\ARMCC\bin\ArmCC" --Via ".\obj\clock.__i"
"d:\Keil_v473\ARM\ARMCC\bin\ArmCC" --Via ".\obj\termostat.__i"
"d:\Keil_v473\ARM\ARMCC\bin\ArmCC" --Via ".\obj\termometer.__i"
"d:\Keil_v473\ARM\ARMCC\bin\ArmCC" --Via ".\obj\adc.__i"
"d:\Keil_v473\ARM\ARMCC\bin\ArmCC" --Via ".\obj\dac.__i"
"d:\Keil_v473\ARM\ARMCC\bin\ArmCC" --Via ".\obj\rtc_svc.__i"
"d:\Keil_v473\ARM\ARMCC\bin\ArmCC" --Via ".\obj\spi_plis.__i"
"d:\Keil_v473\ARM\ARMCC\bin\ArmCC" --Via ".\obj\i2c_mc.__i"
"d:\Keil_v473\ARM\ARMCC\bin\ArmCC" --Via ".\obj\spi_lpc22xx_custom.__i"
"d:\Keil_v473\ARM\ARMCC\bin\ArmCC" --Via ".\obj\fs_spi_flashprg.__i"
"d:\Keil_v473\ARM\ARMCC\bin\ArmCC" --Via ".\obj\retarget.__i"
"d:\Keil_v473\ARM\ARMCC\bin\ArmLink" --Via ".\obj\..\Output\firmware.lnp"
m5.bat FLIGHT_R_2_1
d:\work\C++\util\backup.exe d:\work\OLO\SOLO2_FW\SOLO2_FLIGHT_V2_1\RTX\files.lst /g /z
