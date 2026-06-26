# Toolchain setup
AS := nasm
CC := gcc
LD := ld

# Directory configuration
BIN_DIR := bin
ISO_DIR := keios
INC_DIR := include
SRC_DIR := src

# Target files
LDSCRIPT   := $(SRC_DIR)/linker.ld
KERNEL_BIN := $(BIN_DIR)/keios.bin
ISO_IMAGE  := keios.iso

# Build flags
ASFLAGS := -f elf32
CFLAGS  := -m32 -march=i686 -ffreestanding -nostdlib -O2 -Wall -Wextra -fno-exceptions -std=c23 -I $(INC_DIR) -MMD -MP
LDFLAGS := -m elf_i386 -static -T $(LDSCRIPT)

# Recursively find all .c and .asm files anywhere inside SRC_DIR
C_SRCS   := $(shell find $(SRC_DIR) -type f -name '*.c')
ASM_SRCS := $(shell find $(SRC_DIR) -type f -name '*.asm')

# Map the source files to object files in BIN_DIR
C_OBJS   := $(patsubst $(SRC_DIR)/%.c, $(BIN_DIR)/%.o, $(C_SRCS))
ASM_OBJS := $(patsubst $(SRC_DIR)/%.asm, $(BIN_DIR)/%.o, $(ASM_SRCS))

OBJS := $(C_OBJS) $(ASM_OBJS)
DEPS := $(OBJS:.o=.d)

# Build rules
.PHONY: all clean

all: $(ISO_IMAGE)

$(ISO_IMAGE): $(KERNEL_BIN) grub.cfg
	@echo ">>> Building ISO: $@"
	@mkdir -p $(ISO_DIR)/boot/grub
	@cp grub.cfg $(ISO_DIR)/boot/grub/
	@cp $(KERNEL_BIN) $(ISO_DIR)/boot/
	@grub-mkrescue -o $@ $(ISO_DIR)

$(KERNEL_BIN): $(OBJS) $(LDSCRIPT)
	@echo ">>> Linking kernel: $@"
	@mkdir -p $(dir $@)
	@$(LD) $(LDFLAGS) -o $@ $(OBJS)

# Generic pattern rule to compile any .c file
$(BIN_DIR)/%.o: $(SRC_DIR)/%.c
	@echo ">>> Compiling C: $<"
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@

# Generic pattern rule to assemble any .asm file
$(BIN_DIR)/%.o: $(SRC_DIR)/%.asm
	@echo ">>> Assembling: $<"
	@mkdir -p $(dir $@)
	@$(AS) $(ASFLAGS) $< -o $@

clean:
	@echo ">>> Cleaning up build files"
	@rm -rf $(BIN_DIR) $(ISO_DIR) $(ISO_IMAGE)

# Include the generated .d files
-include $(DEPS)