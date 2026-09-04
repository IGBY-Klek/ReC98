@echo off
setlocal

rem ====== 1) 改成你的实际路径 ======
set DEVKIT=D:\ReC98DevKit
rem ================================

rem Turbo C++ 4.0J
set PATH=%DEVKIT%\TC4J\BIN;%PATH%
set INCLUDE=%DEVKIT%\TC4J\INCLUDE
set LIB=%DEVKIT%\TC4J\LIB

rem Turbo Assembler 5.0 (按你的包结构，两个都加上最稳)
set PATH=%DEVKIT%;%DEVKIT%\TASM;%DEVKIT%\TASM\BIN;%PATH%

echo.
echo [ReC98 local build env]
echo DEVKIT  = %DEVKIT%
echo INCLUDE = %INCLUDE%
echo LIB     = %LIB%
echo.

where tcc
if errorlevel 1 (
  echo [ERROR] tcc not found in PATH
  goto fail
)

where tlink
if errorlevel 1 (
  echo [ERROR] tlink not found in PATH
  goto fail
)

where tasm32
if errorlevel 1 (
  echo [ERROR] tasm32 not found in PATH
  goto fail
)

if not exist "%INCLUDE%\stdio.h" (
  echo [ERROR] Missing stdio.h in %INCLUDE%
  goto fail
)

if not exist "%LIB%\c0t.obj" (
  echo [ERROR] Missing c0t.obj in %LIB%
  goto fail
)

echo [OK] Toolchain paths look good.
echo.

rem 从脚本所在目录运行原 build.bat
pushd "%~dp0"
call build.bat
set BUILD_RC=%ERRORLEVEL%
popd

echo.
if "%BUILD_RC%"=="0" (
  echo [SUCCESS] build.bat finished successfully.
) else (
  echo [FAILED] build.bat exited with code %BUILD_RC%.
)

pause
exit /b %BUILD_RC%

:fail
echo.
echo 请先修正 DEVKIT 路径或开发包内容，再重试。
pause
exit /b 1