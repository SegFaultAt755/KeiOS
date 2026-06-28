@echo off
setlocal enabledelayedexpansion
::initializing variables
set "namu=" 
::searching for folders
for /F "usebackq delims=" %%A in (`DIR /B /A:d ".\src"`) do ( 
	if not exist .\bin\%%A (mkdir bin\%%A)
	echo(
    echo Folder : %%A
	::searching for files
	for %%o in (.\src\%%A\*) do ( 
		if not "%%~no" == "organize" (
			if "%%~xo" == ".c" (
				i686-elf-gcc -m32 -march=i686 -ffreestanding -nostdlib -O2 -Wall -Wextra -fno-exceptions -std=c23 -I include -MMD -MP -c "src\%%A\%%~nxo" -o "bin\%%A\%%~no.o" 
				echo %%~nxo
			)
			if "%%~xo" == ".asm" (
				nasm -f elf32 src\%%A\%%~nxo -o bin\%%A\%%~no.o 
				echo %%~nxo
			)
			::storing file path together
			set "namu=!namu! bin\%%A\%%~no.o " 
		)
	)
)
::adding the stored file path
i686-elf-ld -m elf_i386 -static -T src\linker.ld -o bin\keios.bin %namu% 
qemu-system-i386 -kernel bin\keios.bin
pause
