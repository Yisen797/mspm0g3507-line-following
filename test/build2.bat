@echo off
set PATH=E:in;%PATH%
set SDK=D:电赛资源\TIMSPM0工具链\mspm0_sdk_2_11_00_07
set LIBC=D:电赛资源\TIMSPM0工具链rm-none-eabi
if not exist build mkdir build
E:inrm-none-eabi-gcc -mcpu=cortex-m0plus -mthumb -Os -std=gnu11 -D__MSPM0G3507__ -DDEVICE_MSPM0G3507 -Dgcc -I"