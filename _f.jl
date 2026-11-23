device MSPM0G3507
si SWD
speed 4000
r
h
erase 0x00000000,0x0001FFFF
loadbin %TOOLS%/mspm0_lib/fw.bin 0x00000000
r
g
q
