AS := nasm
CC := gcc
LD := ld

SRC_DIR := src
BIN_DIR := bin
ISO_DIR := isodir

LDSCRIPT   := $(SRC_DIR)/linker.ld
KERNEL_BIN := $(BIN_DIR)/keios.bin
ISO_IMAGE  := keios.iso

ASFLAGS := -f elf32
CFLAGS  := -m32 -march=i686 -ffreestanding -nostdlib -O2 -Wall -Wextra -fno-exceptions -std=c23
LDFLAGS := -m elf_i386 -static -T $(LDSCRIPT)

KERNEL_SRCS := $(wildcard $(SRC_DIR)/kernel/*.c)
KERNEL_OBJS := $(patsubst $(SRC_DIR)/kernel/%.c, $(BIN_DIR)/kernel/%.o, $(KERNEL_SRCS))

BOOT_SRC := $(SRC_DIR)/boot/boot.asm
BOOT_OBJ := $(BIN_DIR)/boot/boot.o

OBJS := $(BOOT_OBJ) $(KERNEL_OBJS)

.PHONY: all clean
all: $(ISO_IMAGE)

$(ISO_IMAGE): $(KERNEL_BIN) grub.cfg
	@echo ">>> Building ISO: $@"
	@mkdir -p $(ISO_DIR)/boot/grub
	@cp grub.cfg $(ISO_DIR)/boot/grub/
	@cp $(KERNEL_BIN) $(ISO_DIR)/boot/
	@grub-mkrescue -o $@ $(ISO_DIR) 2>/dev/null

$(KERNEL_BIN): $(OBJS) $(LDSCRIPT)
	@echo ">>> Linking kernel: $@"
	@mkdir -p $(dir $@)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

$(BIN_DIR)/kernel/%.o: $(SRC_DIR)/kernel/%.c
	@echo ">>> Compiling: $<"
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR)/boot/%.o: $(SRC_DIR)/boot/%.asm
	@echo ">>> Assembling: $<"
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@

clean:
	@echo ">>> Cleaning up build files"
	rm -rf $(BIN_DIR) $(ISO_DIR) $(ISO_IMAGE)
