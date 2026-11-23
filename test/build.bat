@echo off
setlocal
set TIC=D:\ccs\tools\compiler\ti-cgt-armllvm_5.1.1.LTS
set SDK=%TOOLS%\TIMSPM0工具链\mspm0_sdk_2_11_00_07

rmdir /s /q build 2>nul
mkdir build

set CFLAGS=-mcpu=cortex-m0plus -march=thumbv6m -mthumb -mfloat-abi=soft -O2 -Wall -D__MSPM0G3507__ -DDEVICE_MSPM0G3507 -I%SDK%\source -I%SDK%\source\ti\devices\msp\m0p -I%SDK%\source\third_party\CMSIS\Core\Include -I%SDK%\source\ti\driverlib -I%TIC%\include

echo === Compiling ===
%TIC%\bin\tiarmclang %CFLAGS% -c blink.c -o build\blink.o || goto err
%TIC%\bin\tiarmclang %CFLAGS% -c ti_msp_dl_config.c -o build\config.o || goto err
%TIC%\bin\tiarmclang %CFLAGS% -c %SDK%\source\ti\devices\msp\m0p\startup_system_files\ticlang\startup_mspm0g350x_ticlang.c -o build\startup.o || goto err

echo === Linking ===
%TIC%\bin\tiarmclang -Wl,-u,_c_int00 build\blink.o build\config.o build\startup.o %SDK%\examples\nortos\LP_MSPM0G3507\driverlib\empty_non_sysconfig\ticlang\mspm0g3507.cmd -Wl,--rom_model -l%SDK%\source\ti\driverlib\lib\ticlang\m0p\mspm0g1x0x_g3x0x\driverlib.a -L%TIC%\lib -llibc.a -o build\blink.out || goto err

echo === Hex ===
%TIC%\bin\tiarmhex --romwidth=16 --intel build\blink.out -o build\blink.hex || goto err

echo ============================================
echo  BUILD SUCCESS - open web flasher to flash
echo  File: build\blink.hex
echo ============================================
start https://openkits-wiki.easyeda.com/storage/html/mspm0-web-flasher/index.html
goto end

:err
echo BUILD FAILED
:end
