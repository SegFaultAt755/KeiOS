AS = nasm
CC = gcc
LD = ld

ASFLAGS = -f elf32
CFLAGS = -m32 -march=i686 -nostdinc -nostdlib -O2 -Wall -Wextra -Wno-main \
         -fno-exceptions -fno-builtin -nostartfiles -nodefaultlibs -std=c23 -g -fno-pic -fno-pie
LDFLAGS = -T link.ld -m elf_i386 -no-pie

PROG_NAME = init
OUT_DIR ?= ../../bin/userspace/$(PROG_NAME)

default: all
all: bc basm biasm
	@mkdir -p $(OUT_DIR)/src
	@echo ">>> [CC/AS] User-space compiling: program '$(PROG_NAME)'"
	@$(LD) $(LDFLAGS) -o $(OUT_DIR)/$(PROG_NAME).bin $(OUT_DIR)/src/entry.o $(OUT_DIR)/src/syscall.o $(OUT_DIR)/src/main.o

basm: src/entry.asm 
	@mkdir -p $(OUT_DIR)/src
	@$(AS) $(ASFLAGS) src/entry.asm -o $(OUT_DIR)/src/entry.o

biasm: src/syscall.asm 
	@mkdir -p $(OUT_DIR)/src
	@$(AS) $(ASFLAGS) src/syscall.asm -o $(OUT_DIR)/src/syscall.o

bc: src/main.c
	@mkdir -p $(OUT_DIR)/src
	@$(CC) $(CFLAGS) -c src/main.c -o $(OUT_DIR)/src/main.o
