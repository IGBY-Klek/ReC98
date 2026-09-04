@echo off
set DEVKIT=D:\ReC98DevKit
set PATH=%DEVKIT%\TC4J\BIN;%DEVKIT%;%DEVKIT%\TASM;%DEVKIT%\TASM\BIN;%PATH%
set INCLUDE=%DEVKIT%\TC4J\INCLUDE
set LIB=%DEVKIT%\TC4J\LIB
cd /d D:\ReC98Deb3
del /q bin\th04\main.exe bin\th05\main.exe 2>nul
copy /y th04\main\player\render.asm render.asm.touch >nul
call build.bat > build_force.log 2>&1
echo BUILD_RC=%ERRORLEVEL% >> build_force.log
