# ============================================================================
# MSPM0G3507 三轮寻迹小车 — GCC Makefile
# ============================================================================
# ARM GCC:  %ARM_GCC%/bin/ (v15.2.1)
# SDK:      %TOOLS%/TIMSPM0工具链/mspm0_sdk_2_11_00_07
# Libs:     %TOOLS%/mspm0_lib/ (无中文路径, 链接器需要)
# ============================================================================

CROSS   := %ARM_GCC%/bin/arm-none-eabi-
CC      := $(CROSS)gcc
OBJCOPY := $(CROSS)objcopy
SIZE    := $(CROSS)size

CPU     := -mcpu=cortex-m0plus -mthumb -mfloat-abi=soft
OPT     := -Os

MSPM0_SDK   := %TOOLS%/TIMSPM0工具链/mspm0_sdk_2_11_00_07
SDK_SOURCE  := $(MSPM0_SDK)/source
SDK_DEVICE  := $(SDK_SOURCE)/ti/devices/msp/m0p
SDK_CMSIS   := $(SDK_SOURCE)/third_party/CMSIS/Core/Include
SDK_DRVLIB  := $(SDK_SOURCE)/ti/driverlib

LIBC_SYSROOT := %TOOLS%/TIMSPM0工具链/arm-none-eabi

# 无中文路径库目录 (链接器不支持UTF-8路径)
LIBDIR       := %TOOLS%/mspm0_lib

PROJECT     := tracker_mspm0
SRC_DIR     := src
BUILD_DIR   := build

# 源文件
C_SOURCES   := $(wildcard $(SRC_DIR)/*.c)
STARTUP_SRC := $(SDK_DEVICE)/startup_system_files/gcc/startup_mspm0g350x_gcc.c
STARTUP_OBJ := $(BUILD_DIR)/startup_mspm0g350x_gcc.o
C_OBJECTS   := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(C_SOURCES))
C_OBJECTS   += $(STARTUP_OBJ)
DEPS        := $(C_OBJECTS:.o=.d)

DEFINES := -D__MSPM0G3507__ -DDEVICE_MSPM0G3507 -Dgcc

INCLUDES := -I$(SRC_DIR) \
            -I$(SDK_SOURCE) \
            -I$(SDK_DEVICE) \
            -I$(SDK_CMSIS) \
            -I$(SDK_DRVLIB) \
            -isystem $(LIBC_SYSROOT)/include

CFLAGS := $(CPU) $(OPT) -g3 -Wall \
          -ffunction-sections -fdata-sections -fno-common \
          -std=gnu11 $(DEFINES) $(INCLUDES)

# 链接: -nostdlib + 手动指定所有库
LDFLAGS := $(CPU) -nostdlib -Wl,--gc-sections \
           -T $(LIBDIR)/mspm0g3507.lds
LIBS    := $(LIBDIR)/driverlib.a \
           $(LIBDIR)/thumb/v6-m/nofp/libc_nano.a \
           $(LIBDIR)/thumb/v6-m/nofp/libm.a \
           /e/lib/gcc/arm-none-eabi/15.2.1/thumb/v6-m/nofp/libgcc.a

ELF := $(BUILD_DIR)/$(PROJECT).elf
BIN := $(BUILD_DIR)/$(PROJECT).bin
HEX := $(BUILD_DIR)/$(PROJECT).hex

# J-Link 烧录
JLINK := %TOOLS%/J-Link资源/JLink_V810e/JLink.exe
JLINK_SCRIPT := flash.jlink

.PHONY: all clean flash

all: $(BUILD_DIR) $(ELF) $(BIN) $(HEX)
	@echo "============================================"
	@echo " BUILD SUCCESS"
	@echo "============================================"
	$(SIZE) $(ELF)
	@echo "  Flash: $(BIN)"

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "  CC    $<"
	@$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(STARTUP_OBJ): $(STARTUP_SRC)
	@echo "  CC    startup"
	@$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(ELF): $(C_OBJECTS)
	@echo "  LD    $(ELF)"
	@$(CC) $(C_OBJECTS) $(LDFLAGS) $(LIBS) -o $@

$(BIN): $(ELF)
	@echo "  BIN   $(BIN)"
	@$(OBJCOPY) -O binary $< $@

$(HEX): $(ELF)
	@echo "  HEX   $(HEX)"
	@$(OBJCOPY) -O ihex $< $@

flash: $(BIN)
	@echo "Flashing via J-Link..."
	@cp $(BIN) %TOOLS%/mspm0_lib/fw.bin
	@printf 'device MSPM0G3507\nsi SWD\nspeed 4000\nr\nh\nloadbin %TOOLS%/mspm0_lib/fw.bin 0x00000000\nr\ng\nq\n' > $(BUILD_DIR)/_flash.jlink
	@"$(JLINK)" -CommanderScript $(BUILD_DIR)/_flash.jlink
	@rm -f $(BUILD_DIR)/_flash.jlink

clean:
	@rm -rf $(BUILD_DIR)
	@echo "Cleaned."

-include $(DEPS)
