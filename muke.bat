@echo off
setlocal enabledelayedexpansion
set "namu=" ::initializing variables
for /F "usebackq delims=" %%A in (`DIR /B /A:d ".\src"`) do ( ::searching for folders
	if not exist .\bin\%%A (mkdir bin\%%A)
	echo(
    echo Folder : %%A
	for %%o in (.\src\%%A\*) do ( ::searching for files
		if not "%%~no" == "organize" (
			if "%%~xo" == ".c" (
				i686-elf-gcc -m32 -march=i686 -ffreestanding -nostdlib -O2 -Wall -Wextra -fno-exceptions -std=c23 -I include -MMD -MP -c "src\%%A\%%~nxo" -o "bin\%%A\%%~no.o" 
				echo %%~nxo
			)
			if "%%~xo" == ".asm" (
				nasm -f elf32 src\%%A\%%~nxo -o bin\%%A\%%~no.o 
				echo %%~nxo
			)
			set "namu=!namu! bin\%%A\%%~no.o " ::storing file path together
		)
	)
)
i686-elf-ld -m elf_i386 -static -T src\linker.ld -o bin\keios.bin %namu% ::adding the stored file path
qemu-system-i386 -kernel bin\keios.bin
pause
