# HelixRT Makefile for Teensy 4.1 (IMXRT1062)

# Project name
PROJECT = helixrt

# Toolchain
TOOLCHAIN = arm-none-eabi-
CC = $(TOOLCHAIN)gcc
AS = $(TOOLCHAIN)gcc
LD = $(TOOLCHAIN)gcc
OBJCOPY = $(TOOLCHAIN)objcopy
OBJDUMP = $(TOOLCHAIN)objdump
SIZE = $(TOOLCHAIN)size

# Directories
SRC_DIR = src
HAL_DIR = hal
KERNEL_DIR = kernel
BUILD_DIR = build

# CPU/MCU settings
CPU = -mcpu=cortex-m7
FPU = -mfpu=fpv5-d16 -mfloat-abi=hard
MCU = $(CPU) -mthumb $(FPU)

# Compiler flags
CFLAGS = $(MCU)
CFLAGS += -O2 -g3
CFLAGS += -Wall -Wextra -Werror
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -fno-common
CFLAGS += -ffreestanding
CFLAGS += -nostdlib
CFLAGS += -std=c11
CFLAGS += -I.

# Assembler flags
ASFLAGS = $(MCU)
ASFLAGS += -g3
ASFLAGS += -x assembler-with-cpp

# Linker flags
LDFLAGS = $(MCU)
LDFLAGS += -T linker.ld
LDFLAGS += -Wl,--gc-sections
LDFLAGS += -Wl,-Map=$(BUILD_DIR)/$(PROJECT).map
LDFLAGS += -nostartfiles
LDFLAGS += -nostdlib
LDFLAGS += --specs=nosys.specs

# Source files
C_SOURCES = \
	$(SRC_DIR)/startup.c \
	$(SRC_DIR)/main.c \
	$(SRC_DIR)/syscall.c \
	$(KERNEL_DIR)/kernel.c \
	$(KERNEL_DIR)/scheduler.c \
	$(KERNEL_DIR)/timer.c \
	$(KERNEL_DIR)/sync/critical.c \
	$(KERNEL_DIR)/sync/semaphore.c \
	$(KERNEL_DIR)/sync/mutex.c \
	$(KERNEL_DIR)/sync/queue.c \
	$(KERNEL_DIR)/sync/event.c

# Assembly sources (if any)
ASM_SOURCES = \
	$(KERNEL_DIR)/context.s

# Object files
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))

# VPATH for source files
vpath %.c $(sort $(dir $(C_SOURCES)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

# Default target
all: $(BUILD_DIR)/$(PROJECT).elf $(BUILD_DIR)/$(PROJECT).hex $(BUILD_DIR)/$(PROJECT).bin

# Create build directory
$(BUILD_DIR):
	mkdir -p $@

# Compile C files
$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	@echo "CC $<"
	@$(CC) $(CFLAGS) -c $< -o $@

# Assemble .s files
$(BUILD_DIR)/%.o: %.s | $(BUILD_DIR)
	@echo "AS $<"
	@$(AS) $(ASFLAGS) -c $< -o $@

# Link
$(BUILD_DIR)/$(PROJECT).elf: $(OBJECTS)
	@echo "LD $@"
	@$(LD) $(LDFLAGS) $(OBJECTS) -o $@
	@$(SIZE) $@

# Create HEX file
$(BUILD_DIR)/$(PROJECT).hex: $(BUILD_DIR)/$(PROJECT).elf
	@echo "OBJCOPY $@"
	@$(OBJCOPY) -O ihex $< $@

# Create BIN file
$(BUILD_DIR)/$(PROJECT).bin: $(BUILD_DIR)/$(PROJECT).elf
	@echo "OBJCOPY $@"
	@$(OBJCOPY) -O binary $< $@

# Disassembly
disasm: $(BUILD_DIR)/$(PROJECT).elf
	$(OBJDUMP) -d -S $< > $(BUILD_DIR)/$(PROJECT).dis

# Flash using teensy_loader_cli (if available)
flash: $(BUILD_DIR)/$(PROJECT).hex
	teensy_loader_cli --mcu=TEENSY41 -w -v $<

# Clean
clean:
	rm -rf $(BUILD_DIR)

# Phony targets
.PHONY: all clean flash disasm

# Print variables (for debugging)
print-%:
	@echo $* = $($*)
