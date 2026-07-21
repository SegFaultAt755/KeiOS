@echo off
setlocal enabledelayedexpansion

REM Toolchain configuration
set "TOOLCHAIN_PREFIX=i686-elf-"
set "CC=%TOOLCHAIN_PREFIX%gcc"
set "AS=nasm"
set "LD=%TOOLCHAIN_PREFIX%ld"

REM Load custom configuration
if exist "config.bat" call "config.bat"

REM Directories & Target configurations
set "BIN_DIR=bin"
set "ISO_DIR=keios"
set "INC_DIR=include"
set "SRC_DIR=src"
set "RUST_DIR=rust"

set "LDSCRIPT=linker.ld"
set "KERNEL_BIN=%BIN_DIR%\keios.elf"
set "ISO_IMAGE=keios.iso"

set "RUST_TARGET=i686-unknown-none"
set "RUST_CRATES=drivers"

REM Default flags
set "ASFLAGS=-f elf32"
set "CFLAGS=-m32 -march=i686 -ffreestanding -nostdlib -O2 -Wall -Wextra -fno-exceptions -std=c23 -I %INC_DIR% -MMD -MP"
set "LDFLAGS=-m elf_i386 -static -T %LDSCRIPT%"

REM Store root working directory path
set "WORK_DIR=%CD%"
if "!WORK_DIR:~-1!"=="\" set "WORK_DIR=!WORK_DIR:~0,-1!"

REM Parse command line arguments
set "TARGET=all"
set "CUSTOM_DEFINES="

for %%a in (%*) do (
    set "arg=%%~a"
    if /i "!arg!"=="all" set "TARGET=all"
    if /i "!arg!"=="clean" set "TARGET=clean"
    if /i "!arg!"=="config" set "TARGET=config"
    if /i "!arg!"=="help" set "TARGET=help"
    if /i "!arg:~0,2!"=="D=" (
        set "defs=!arg:~2!"
        for %%d in (!defs!) do set "CUSTOM_DEFINES=!CUSTOM_DEFINES! -D%%d"
    )
)

if defined CUSTOM_DEFINES (
    set "ASFLAGS=%ASFLAGS%%CUSTOM_DEFINES%"
    set "CFLAGS=%CFLAGS%%CUSTOM_DEFINES%"
)

REM Route target command
if /i "%TARGET%"=="all" goto :all
if /i "%TARGET%"=="clean" goto :clean
if /i "%TARGET%"=="config" goto :config
if /i "%TARGET%"=="help" goto :help

echo Unknown target: %TARGET%
goto :help

REM Build rules

:all
echo >>> [BUILD] Starting KeiOS build...
set "OBJS="

REM Recursively locate and compile C and ASM sources
if exist "%SRC_DIR%" (
    for /r "%SRC_DIR%" %%f in (*.c) do call :compile_c "%%f"
    for /r "%SRC_DIR%" %%f in (*.asm) do call :compile_asm "%%f"
)

REM Compile rust crates
call :compile_rust

REM Link kernel binary
call :link_kernel
if errorlevel 1 goto :error

REM Generate bootable ISO
call :build_iso
if errorlevel 1 goto :error

echo >>> Build completed successfully!
goto :eof

:compile_c
set "SRC_FILE=%~1"
set "FILE_DIR=%~dp1"
set "FILE_NAME=%~n1"

REM Extract relative path inside src
set "REL_DIR=!FILE_DIR:%WORK_DIR%\%SRC_DIR%\=!"
if "!REL_DIR!"=="!FILE_DIR!" set "REL_DIR="

set "OBJ_DIR=%BIN_DIR%\!REL_DIR!"
if not exist "!OBJ_DIR!" mkdir "!OBJ_DIR!"
set "OBJ_FILE=!OBJ_DIR!!FILE_NAME!.o"

REM Target specific flag rule
set "LOCAL_CFLAGS=%CFLAGS%"
if /i "!REL_DIR!!FILE_NAME!"=="libkern\math" set "LOCAL_CFLAGS=%LOCAL_CFLAGS% -msse"

echo >>> [CC]  Compiling: !REL_DIR!!FILE_NAME!.c
%CC% !LOCAL_CFLAGS! -c "%SRC_FILE%" -o "!OBJ_FILE!"
if errorlevel 1 goto :error

set "OBJS=!OBJS! "!OBJ_FILE!""
goto :eof

:compile_asm
set "SRC_FILE=%~1"
set "FILE_DIR=%~dp1"
set "FILE_NAME=%~n1"

set "REL_DIR=!FILE_DIR:%WORK_DIR%\%SRC_DIR%\=!"
if "!REL_DIR!"=="!FILE_DIR!" set "REL_DIR="

set "OBJ_DIR=%BIN_DIR%\!REL_DIR!"
if not exist "!OBJ_DIR!" mkdir "!OBJ_DIR!"
set "OBJ_FILE=!OBJ_DIR!!FILE_NAME!.o"

echo >>> [AS]  Assembling: !REL_DIR!!FILE_NAME!.asm
%AS% %ASFLAGS% "%SRC_FILE%" -o "!OBJ_FILE!"
if errorlevel 1 goto :error

set "OBJS=!OBJS! "!OBJ_FILE!""
goto :eof

:compile_rust
if exist "%RUST_DIR%\Cargo.toml" (
    echo >>> [RS]  Compiling Rust crate: %RUST_CRATES%
    pushd "%RUST_DIR%"
    cargo build --release --target %RUST_TARGET% -p %RUST_CRATES%
    if errorlevel 1 (
        popd
        goto :error
    )
    popd
    for %%r in ("%RUST_DIR%\target\%RUST_TARGET%\release\*.a") do (
        set "OBJS=!OBJS! "%%~fr""
    )
)
goto :eof

:link_kernel
echo >>> [LD]  Linking kernel binary: %KERNEL_BIN%
if not exist "%BIN_DIR%" mkdir "%BIN_DIR%"
%LD% %LDFLAGS% -o "%KERNEL_BIN%" !OBJS!
goto :eof

:build_iso
echo >>> [ISO] Generating bootable image: %ISO_IMAGE%
if not exist "%ISO_DIR%\boot\grub" mkdir "%ISO_DIR%\boot\grub"
copy /y "%KERNEL_BIN%" "%ISO_DIR%\boot\keios.elf" >nul

REM Attempt grub-mkrescue first
where grub-mkrescue >nul 2>&1
if !errorlevel! equ 0 (
    if exist "grub.cfg" copy /y "grub.cfg" "%ISO_DIR%\boot\grub\grub.cfg" >nul
    grub-mkrescue -o "%ISO_IMAGE%" "%ISO_DIR%" 2>nul
) else (
    REM Fallback to xorriso and legacy GRUB files
    if exist "menu.lst" copy /y "menu.lst" "%ISO_DIR%\boot\grub" >nul
    if exist "stage1" copy /y "stage1" "%ISO_DIR%\boot\grub" >nul
    if exist "stage2" copy /y "stage2" "%ISO_DIR%\boot\grub" >nul
    if exist "grub.cfg" copy /y "grub.cfg" "%ISO_DIR%\boot\grub\grub.cfg" >nul
    xorriso -outdev "%ISO_IMAGE%" -blank as_needed -map "%ISO_DIR%" / -boot_image grub bin_path=/boot/grub/stage1
)
goto :eof

REM Utility subcommands

:config
if not exist "config.bat" (
    echo >>> Generating default config.bat...
    (
        echo @echo off
        echo REM Custom KeiOS Build Configuration
        echo set "TOOLCHAIN_PREFIX=%TOOLCHAIN_PREFIX%"
        echo set "ASFLAGS=%ASFLAGS%"
        echo set "CFLAGS=%CFLAGS%"
        echo set "LDFLAGS=%LDFLAGS%"
    ) > config.bat
    echo Configuration saved to config.bat
) else (
    echo >>> WARNING: 'config.bat' already exists. Generation skipped to protect custom settings.
    echo >>> NOTE: To reset all data, delete the configuration file and re-run 'muke.bat config'
)
goto :eof

:clean
echo >>> [CLEAN] Removing build directories...
if exist "%BIN_DIR%" rmdir /s /q "%BIN_DIR%"
if exist "%ISO_DIR%" rmdir /s /q "%ISO_DIR%"
if exist "isomer" rmdir /s /q "isomer"
if exist "%ISO_IMAGE%" del /q "%ISO_IMAGE%"
if exist "%RUST_DIR%\Cargo.toml" (
    pushd "%RUST_DIR%"
    cargo clean
    popd
)
goto :eof

:help
echo KeiOS Build System (Batch Edition)
echo     muke.bat all        - Build the OS ISO image (default)
echo     muke.bat clean      - Remove build artifacts
echo     muke.bat config     - Generate a customizable config.bat file
echo     muke.bat help       - Show this help message
echo     muke.bat D="FLAG=1" - Pass custom preprocessor definitions
goto :eof

:error
echo >>> [ERROR] Build failed! Aborting.
exit /b 1
