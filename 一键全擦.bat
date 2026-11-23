@echo off
chcp 65001 >nul
title MSPM0G3507 天猛星 全片擦除
echo ==============================================
echo   MSPM0G3507 天猛星 全片擦除工具
echo ==============================================
echo.
echo   [接线要求]
echo     3V3   - 3V3
echo     GND   - GND
echo     SWDIO - PA19
echo     SWCLK - PA20
echo.
echo   [注意]
echo     全擦会清空程序+恢复BSL, 擦完需重新烧录
echo     若擦完连不上: 断电30秒再上电
echo.
echo   按任意键开始全擦...
pause >nul
echo.
echo   正在查找J-Link...

set "JLINK="
set "SCRIPT=%~dp0一键全擦.jlink"

rem 1. 优先同目录 JLink.exe
if exist "%~dp0JLink.exe" set "JLINK=%~dp0JLink.exe"

rem 2. 常见安装路径
if not defined JLINK if exist "%TOOLS%\J-Link资源\JLink_V810e\JLink.exe" set "JLINK=%TOOLS%\J-Link资源\JLink_V810e\JLink.exe"

rem 3. 系统PATH查找
if not defined JLINK where JLink.exe >nul 2>nul && set "JLINK=JLink.exe"

if not defined JLINK (
    echo.
    echo   [错误] 找不到 JLink.exe
    echo   请把 JLink.exe 放到本目录, 或安装J-Link软件
    echo.
    pause
    exit /b 1
)

echo   找到: %JLINK%
echo   正在全片擦除...
echo.

"%JLINK%" -device MSPM0G3507 -if SWD -speed 4000 -CommanderScript "%SCRIPT%"

echo.
echo ==============================================
echo   擦除完成(如上方无报错)。
echo   断电30秒后重新上电即可。
echo ==============================================
echo.
pause
