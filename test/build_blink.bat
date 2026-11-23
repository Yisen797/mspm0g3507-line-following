@echo off
chcp 65001 >nul
setlocal
set PATH=%ARM_GCC%\bin;%PATH%
set GCC=%ARM_GCC%\bin\arm-none-eabi-gcc
set OBJCOPY=%ARM_GCC%\bin\arm-none-eabi-objcopy
set SDKROOT=%TOOLS%\TIMSPM0工具链
set SDK=%SDKROOT%\mspm0_sdk_2_11_00_07

if not exist build mkdir build

echo === Compiling blink ===
%GCC% -mcpu=cortex-m0plus -mthumb -Os -std=gnu11 -D__MSPM0G3507__ -DDEVICE_MSPM0G3507 -Dgcc ^
  -I"%SDK%\source" -I"%SDK%\source\ti\devices\msp\m0p" ^
  -I"%SDK%\source\third_party\CMSIS\Core\Include" ^
  -I"%SDK%\source\ti\driverlib" ^
  -isystem "%SDKROOT%\arm-none-eabi\include" ^
  -c blink.c -o build\blink.o
if %ERRORLEVEL% NEQ 0 goto err

%GCC% -mcpu=cortex-m0plus -mthumb -Os -std=gnu11 -D__MSPM0G3507__ -DDEVICE_MSPM0G3507 -Dgcc ^
  -I"%SDK%\source" -I"%SDK%\source\ti\devices\msp\m0p" ^
  -I"%SDK%\source\third_party\CMSIS\Core\Include" ^
  -I"%SDK%\source\ti\driverlib" ^
  -isystem "%SDKROOT%\arm-none-eabi\include" ^
  -c "%SDK%\source\ti\devices\msp\m0p\startup_system_files\gcc\startup_mspm0g350x_gcc.c" ^
  -o build\startup.o
if %ERRORLEVEL% NEQ 0 goto err

echo === Linking ===
%GCC% build\blink.o build\startup.o ^
  -mcpu=cortex-m0plus -mthumb -Wl,--gc-sections ^
  -specs=nosys.specs -specs=nano.specs -nostartfiles ^
  -T "%SDK%\source\ti\devices\msp\m0p\linker_files\gcc\mspm0g3507.lds" ^
  -L "%SDK%\source\ti\driverlib\lib\gcc\m0p\mspm0g1x0x_g3x0x" -l:driverlib.a ^
  -L "%SDKROOT%\arm-none-eabi\lib\thumb\v6-m\nofp" -lgcc -lc -lm ^
  -Wl,-Map=build\blink.map ^
  -o build\blink.elf
if %ERRORLEVEL% NEQ 0 goto err

echo === Binary ===
%OBJCOPY% -O binary build\blink.elf build\blink.bin
%OBJCOPY% -O ihex build\blink.elf build\blink.hex

echo ============================================
echo  BUILD SUCCESS - flash build\blink.bin
echo ============================================
goto end

:err
echo BUILD FAILED
:end
pause
