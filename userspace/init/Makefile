AS = nasm
CC = gcc
LD = ld

PROG_NAME = init
SRC_DIRS  ?= src ../stdlib
OUT_DIR   ?= ../../bin/userspace/$(PROG_NAME)
INCLUDE   ?= stdlib

INC_FLAGS := $(addprefix -I,$(INCLUDE))
ASFLAGS = -f elf32
CFLAGS = -m32 -march=i686 -nostdlib -O2 -Wall -Wextra \
         -fno-exceptions -fno-builtin -nostartfiles -std=c23 -fno-pic -fno-pie $(INC_FLAGS)
LDFLAGS = -T link.ld -m elf_i386

C_SRCS   := $(shell find $(SRC_DIRS) -type f -name '*.c')
ASM_SRCS := $(shell find $(SRC_DIRS) -type f -name '*.asm')

C_OBJS   := $(patsubst %.c,$(OUT_DIR)/%.o,$(C_SRCS))
ASM_OBJS := $(patsubst %.asm,$(OUT_DIR)/%.o,$(ASM_SRCS))
ALL_OBJS := $(ASM_OBJS) $(C_OBJS)

.PHONY: all clean

all: $(ALL_OBJS)
	@mkdir -p $(OUT_DIR)
	@echo ">>> [LD]  Linking program '$(PROG_NAME)'"
	@$(LD) $(LDFLAGS) -o $(OUT_DIR)/$(PROG_NAME).bin $(ALL_OBJS)

$(OUT_DIR)/%.o: %.asm
	@echo ">>> [AS]  Assembling: $(PROG_NAME)/$<"
	@mkdir -p $(dir $@)
	@$(AS) $(ASFLAGS) $< -o $@

$(OUT_DIR)/%.o: %.c
	@echo ">>> [CC]  Compiling: $(PROG_NAME)/$<"
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -rf $(OUT_DIR)
