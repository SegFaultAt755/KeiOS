AS = nasm
ASFLAGS = -f elf32

SRC = main

default: all
all: $(SRC).asm
	@echo ">>> [AS]  User-space assembling: $< in $(CURDIR)"
	@$(AS) $(ASFLAGS) $(SRC).asm -o $(SRC).elf
