@echo off
i686-elf-gcc -m32 -march=i686 -ffreestanding -nostdlib -O2 -Wall -Wextra -fno-exceptions -std=c23 -I include -MMD -MP -c src\kernel\kernel.c -o bin\kernel\kernel.o
i686-elf-gcc -m32 -march=i686 -ffreestanding -nostdlib -O2 -Wall -Wextra -fno-exceptions -std=c23 -I include -MMD -MP -c src\kernel\terminal.c -o bin\kernel\terminal.o
i686-elf-gcc -m32 -march=i686 -ffreestanding -nostdlib -O2 -Wall -Wextra -fno-exceptions -std=c23 -I include -MMD -MP -c src\kernel\panic.c -o bin\kernel\panic.o
i686-elf-gcc -m32 -march=i686 -ffreestanding -nostdlib -O2 -Wall -Wextra -fno-exceptions -std=c23 -I include -MMD -MP -c src\cpu\gdt.c -o bin\cpu\gdt.o
i686-elf-gcc -m32 -march=i686 -ffreestanding -nostdlib -O2 -Wall -Wextra -fno-exceptions -std=c23 -I include -MMD -MP -c src\libkern\memory.c -o bin\libkern\memory.o
i686-elf-gcc -m32 -march=i686 -ffreestanding -nostdlib -O2 -Wall -Wextra -fno-exceptions -std=c23 -I include -MMD -MP -c src\libkern\stdio.c -o bin\libkern\stdio.o
i686-elf-gcc -m32 -march=i686 -ffreestanding -nostdlib -O2 -Wall -Wextra -fno-exceptions -std=c23 -I include -MMD -MP -c src\libkern\string.c -o bin\libkern\string.o
nasm -f elf32 src\boot\boot.asm -o bin\boot\boot.o
nasm -f elf32 src\cpu\gdt_flush.asm -o bin\cpu\gdt_flush.o
i686-elf-ld -m elf_i386 -static -T src\linker.ld -o bin\keios.bin bin\boot\boot.o bin\kernel\kernel.o bin\kernel\terminal.o bin\kernel\panic.o bin\cpu\gdt.o bin\libkern\memory.o bin\libkern\stdio.o bin\libkern\string.o
qemu-system-i386 -kernel bin\keios.bin
pause