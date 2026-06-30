@echo off
cd src
set "StartPath=%cd%"
cd ..
setlocal enabledelayedexpansion
REM initializing variables
set "namu=" 
REM searching for folders
for /f "tokens=*" %%f in ('dir /b /ad /s "%StartPath%"') do ( 
    set "FullPath=%%f"
    set "RelativePath=!FullPath:%StartPath%\=!"
	if not exist .\bin\!RelativePath! (mkdir bin\!RelativePath!)
	echo(
    echo !RelativePath!
	for %%o in (.\src\!RelativePath!\*) do ( 
		if not "%%~no" == "organize" (
			if "%%~xo" == ".c" (
				i686-elf-gcc -m32 -march=i686 -ffreestanding -nostdlib -O2 -Wall -Wextra -fno-exceptions -std=c23 -I include -MMD -MP -c "src\!RelativePath!\%%~nxo" -o "bin\!RelativePath!\%%~no.o" 
				echo %%~nxo
			)
			if "%%~xo" == ".asm" (
				nasm -f elf32 src\!RelativePath!\%%~nxo -o bin\!RelativePath!\%%~no.o 
				echo %%~nxo
			)
			REM storing file path together
			set "namu=!namu! bin\!RelativePath!\%%~no.o " 
		)
	)
)
echo Compiling
REM adding the stored file path
i686-elf-ld -m elf_i386 -static -T src\linker.ld -o bin\keios.bin %namu% 
qemu-system-i386 -kernel bin\keios.bin
endlocal
pause
