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

rwildcard = $(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

# Find all .c and .asm files anywhere inside SRC_DIR
C_SRCS   := $(call rwildcard,$(SRC_DIR),*.c)
ASM_SRCS := $(call rwildcard,$(SRC_DIR),*.asm)

# Cross-platform command mapping
ifeq ($(OS),Windows_NT)
    # Windows Native Commands (CMD)
    MKDIR = if not exist $(subst /,\,$1) mkdir $(subst /,\,$1)
    RM_RF = if exist $(subst /,\,$1) rmdir /s /q $(subst /,\,$1)
    RM_F  = if exist $(subst /,\,$1) del /q $(subst /,\,$1)
    CP    = copy /y $(subst /,\,$1) $(subst /,\,$2)
else
    # Linux/macOS POSIX Commands
    MKDIR = mkdir -p $1
    RM_RF = rm -rf $1
    RM_F  = rm -f $1
    CP    = cp $1 $2
endif

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
	@$(call MKDIR,$(ISO_DIR)/boot/grub)
	@$(call CP,grub.cfg,$(ISO_DIR)/boot/grub/grub.cfg)
	@$(call CP,$(KERNEL_BIN),$(ISO_DIR)/boot/$(notdir $(KERNEL_BIN)))
	@grub-mkrescue -o $@ $(ISO_DIR)

$(KERNEL_BIN): $(OBJS) $(LDSCRIPT)
	@echo ">>> Linking kernel: $@"
	@$(call MKDIR,$(dir $@))
	@$(LD) $(LDFLAGS) -o $@ $(OBJS)

# Generic pattern rule to compile any .c file
$(BIN_DIR)/%.o: $(SRC_DIR)/%.c
	@echo ">>> Compiling C: $<"
	@$(call MKDIR,$(dir $@))
	@$(CC) $(CFLAGS) -c $< -o $@

# Generic pattern rule to assemble any .asm file
$(BIN_DIR)/%.o: $(SRC_DIR)/%.asm
	@echo ">>> Assembling: $<"
	@$(call MKDIR,$(dir $@))
	@$(AS) $(ASFLAGS) $< -o $@

clean:
	@echo ">>> Cleaning up build files"
	@$(call RM_RF,$(BIN_DIR))
	@$(call RM_RF,$(ISO_DIR))
	@$(call RM_F,$(ISO_IMAGE))

# Include the generated .d files
-include $(DEPS)