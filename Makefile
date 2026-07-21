# SPDX-License-Identifier: GPLv3
# Copyright (C) 2026 KeiOS Developers

# Toolchain
TOOLCHAIN_PREFIX ?= $(shell which i686-elf-gcc >/dev/null 2>&1 && echo "i686-elf-" || echo "")
AS := nasm
CC := $(TOOLCHAIN_PREFIX)gcc
LD := $(TOOLCHAIN_PREFIX)ld

# Load custom configuration
-include config.mk

# Directory configuration
BIN_DIR := bin
ISO_DIR := keios
INC_DIR := include
SRC_DIR := src
FS_DIR  := rootfs

# Target files
LDSCRIPT   := linker.ld
KERNEL := keios.elf
CPIO       := initramfs.cpio
ISO_IMAGE  := keios.iso

# Custom preprocessor defines
# Usage: make D="DEBUG VERBOSE=1" -> -DDEBUG -DVERBOSE=1
ifneq ($(strip $(D)),)
    CUSTOM_DEFINES := $(foreach def,$(D),-D$(def))
endif

# Build flags
ASFLAGS ?= -f elf32 $(CUSTOM_DEFINES)
CFLAGS  ?= -m32 -march=i686 -ffreestanding -nostdlib -O2 -Wall -Wextra \
           -fno-exceptions -std=c23 -I $(INC_DIR) -MMD -MP $(CUSTOM_DEFINES)
LDFLAGS ?= -m elf_i386 -static -T $(LDSCRIPT)

bin/libkern/math.o: CFLAGS += -msse

# Cross-platform file utilities
ifeq ($(OS),Windows_NT)
    MKDIR = if not exist $(subst /,\,$1) mkdir $(subst /,\,$1)
    RM_RF = if exist $(subst /,\,$1) rmdir /s /q $(subst /,\,$1)
    RM_F  = if exist $(subst /,\,$1) del /q $(subst /,\,$1)
    CP    = copy /y $(subst /,\,$1) $(subst /,\,$2) >nul
else
    MKDIR = mkdir -p $1
    RM_RF = rm -rf $1
    RM_F  = rm -f $1
    CP    = cp $1 $2
endif

# Rust configuration
RUST_TARGET := i686-unknown-none
RUST_DIR    := rust
RUST_CRATES := drivers
RUST_LIBS   := $(patsubst %, $(RUST_DIR)/target/$(RUST_TARGET)/release/lib%.a, $(RUST_CRATES))

# Source and object resolution
rwildcard = $(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

C_SRCS   := $(call rwildcard,$(SRC_DIR),*.c)
ASM_SRCS := $(call rwildcard,$(SRC_DIR),*.asm)

C_OBJS   := $(patsubst $(SRC_DIR)/%.c, $(BIN_DIR)/%.o, $(C_SRCS))
ASM_OBJS := $(patsubst $(SRC_DIR)/%.asm, $(BIN_DIR)/%.o, $(ASM_SRCS))

OBJS := $(C_OBJS) $(ASM_OBJS) $(RUST_LIBS)
DEPS := $(C_OBJS:.o=.d) $(ASM_OBJS:.o=.d)

# Build rules
.PHONY: all clean config help

all: $(ISO_IMAGE)

$(ISO_IMAGE): $(KERNEL) grub.cfg $(CPIO)
	@echo ">>> [ISO] Generating bootable image: $@"
	@$(call MKDIR,$(ISO_DIR)/boot/grub)
	@$(call CP,grub.cfg,$(ISO_DIR)/boot/grub/grub.cfg)
	@$(call CP,$(KERNEL),$(ISO_DIR)/boot/$(notdir $(KERNEL)))
	@$(call CP,$(CPIO),$(ISO_DIR)/boot/$(notdir $(CPIO)))
	@grub-mkrescue -o $@ $(ISO_DIR) 2>/dev/null || (echo "Error: grub-mkrescue failed." && false)

$(CPIO):
	@echo ">>> [IO]  Generating rootfs file: $@"
	@cd $(FS_DIR) && find . -print0 | cpio --null -o --format=newc > ../$(CPIO)

$(KERNEL): $(OBJS) $(LDSCRIPT)
	@echo ">>> [LD]  Linking kernel binary: $@"
	@$(LD) $(LDFLAGS) -o $@ $(OBJS)

$(BIN_DIR)/%.o: $(SRC_DIR)/%.c
	@echo ">>> [CC]  Compiling: $<"
	@$(call MKDIR,$(dir $@))
	@$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR)/%.o: $(SRC_DIR)/%.asm
	@echo ">>> [AS]  Assembling: $<"
	@$(call MKDIR,$(dir $@))
	@$(AS) $(ASFLAGS) $< -o $@

$(RUST_DIR)/target/$(RUST_TARGET)/release/lib%.a:
	@echo ">>> [RS]  Compiling Rust crate: $*"
	@cd $(RUST_DIR) && cargo build --release -p $*

# Generate a local configuration
config:
ifeq ($(wildcard config.mk),)
	@echo ">>> Generating default config.mk..."
	@echo "# Custom KeiOS Build Configuration" > config.mk
	@echo "TOOLCHAIN_PREFIX := $(TOOLCHAIN_PREFIX)" >> config.mk
	@echo "ASFLAGS := $(ASFLAGS)" >> config.mk
	@echo "CFLAGS := $(CFLAGS)" >> config.mk
	@echo "LDFLAGS := $(LDFLAGS)" >> config.mk
	@echo "Configuration saved to config.mk"
else
	@echo ">>> WARNING: 'config.mk' already exists. Generation skipped to protect custom settings."
	@echo ">>> NOTE: To reset all data, first delete the configuration file and re-run 'make config'"
endif

clean:
	@echo ">>> [CLEAN] Removing build directories..."
	@$(call RM_RF,$(BIN_DIR))
	@$(call RM_RF,$(ISO_DIR))
	@$(call RM_F,$(ISO_IMAGE))
	@$(call RM_F,$(KERNEL))
	@$(call RM_F,$(CPIO))
	@if [ -d "$(RUST_DIR)" ]; then cd $(RUST_DIR) && cargo clean; fi

help:
	@echo "KeiOS Build System"
	@echo "    make all          - Build the OS ISO image (default)"
	@echo "    make clean        - Remove build artifacts"
	@echo "    make config       - Generate a customizable config.mk file"
	@echo "    make D=\"FLAG=1\"   - Pass custom preprocessor definitions"

-include $(DEPS)
