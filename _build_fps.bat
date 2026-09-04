@echo off
set DEVKIT=D:\ReC98DevKit
set PATH=%DEVKIT%\TC4J\BIN;%DEVKIT%;%DEVKIT%\TASM;%DEVKIT%\TASM\BIN;%PATH%
set INCLUDE=%DEVKIT%\TC4J\INCLUDE
set LIB=%DEVKIT%\TC4J\LIB
cd /d D:\ReC98Deb3
call build.bat > build_fps.log 2>&1
echo BUILD_RC=%ERRORLEVEL% >> build_fps.log
