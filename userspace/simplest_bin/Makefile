AS = nasm
ASFLAGS = -f elf32

SRC = main
PROG_NAME = simplest_bin

# Hardcoded path, and it must be called from the project's root to avoid errors
OUT_DIR = bin/userspace/$(PROG_NAME)

default: all

all: $(SRC).asm
	@mkdir -p $(OUT_DIR)
	@echo ">>> [AS]  User-space assembling: program '$(PROG_NAME)'"
	@$(AS) $(ASFLAGS) $(SRC).asm -o $(OUT_DIR)/$(SRC).elf
