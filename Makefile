build:
	mkdir -p bin
	mkdir -p bin/bootloader
	mkdir -p bin/kernel
	nasm -f elf32 ./src/bootloader/boot.asm -o ./bin/bootloader/boot.o
	gcc -c ./src/kernel/kernel.c -o ./bin/kernel/kernel.o -ffreestanding -nostdlib -O2 -Wall -Wextra -fno-exceptions -m32 -march=i686
	gcc -c ./src/kernel/terminal.c -o ./bin/kernel/terminal.o -ffreestanding -nostdlib -O2 -Wall -Wextra -fno-exceptions -m32 -march=i686
	ld -m elf_i386 -static -T ./src/linker.ld -o bin/keios.bin ./bin/bootloader/boot.o ./bin/kernel/kernel.o ./bin/kernel/terminal.o
	mkdir -p isodir
	mkdir -p isodir/boot
	mkdir -p isodir/boot/grub
	cp grub.cfg isodir/boot/grub
	cp bin/keios.bin isodir/boot
	grub-mkrescue -o keios.iso isodir
