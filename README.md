# KeiOS

![Architecture: 32-bit i386](https://img.shields.io/badge/Arch-i386-blue) ![License: GPLv3](https://img.shields.io/badge/License-GPLv3-green) ![Experimental](https://img.shields.io/badge/status-experimental-orange) ![In Development](https://img.shields.io/badge/status-in--development-yellow)

> 32-bit lab operating system built to experiment with cross-platform executable support via dual syscall tables, powered by a modern C23 and Rust architecture

*(⚠️ Note: KeiOS is currently in active development...)*

---
## 🗺️ Table of Contents

- [🎯 Project Vision](#-project-vision)
- [🚀 Key Features](#-key-features)
- [📂 Project Structure](#-project-structure)
- [🖥️ Emulating System Environment](#%EF%B8%8F-emulating-system-environment)
- [🛠️ Prerequisites](#%EF%B8%8F-prerequisites)
    - [🐧 Linux (Ubuntu/Debian)](#-linux-ubuntudebian)
    - [🪟 Windows](#-windows)
- [🏃 Build and Run](#-build-and-run)
    - [⚙️ Compile](#1-compile-the-os)
    - [🚀 Launch](#2-launch-in-emulator)
- [🔍 Troubleshooting & Common Pitfalls](#-troubleshooting--common-pitfalls)
- [💬 Final Words](#-final-words)
- [📄 License](#-license)

---
## 🎯 Project Vision

The primary goal of KeiOS is to serve as an experimental lab environment. Its standout feature is the planned implementation of **two distinct syscall tables**, allowing the OS to natively handle and execute both **Windows** and **Linux** binaries within a minimal custom kernel and user space

---
## 🚀 Key Features

* **Hybrid Core Architecture:** Built using modern C23 and Rust (v1.96) with seamless FFI integration for memory safe driver and kernel component development
* **Architecture:** 32-bit (`i386`) operating on a microkernel architecture
* **Dual Syscall Interface:** Purpose built routing system to process system calls from both Windows and Linux executables natively
* **Modular Design:** Clean structural separation between the bootloader, kernel, drivers, and user space code

---
## 📂 Project Structure

`````txt
KeiOS/
├── bin/         # Compiled binary
├── include/     # Main header directory
│   ├── arch/    # Architecture specific header definitions
│   │   └── x86/ # 32-bit control registers, segmentations, and offsets
│   ├── drivers/ # Peripheral configurations (network, audio, display)
│   ├── kernel/  # Microkernel core system
│   ├── libkern/ # Mini C standard library implementations for kernel space
│   └── config.h # Static build configuration toggles
├── rust/        # Rust workspace for memory safe kernel components and drivers
│   ├── .cargo/  # Cargo build configuration and target flags
│   ├── crates/  # Modular Rust crates (e.g., drivers, kernel, syslibs)
│   ├── Cargo.toml             # Workspace root dependency manager
│   ├── i686-unknown-none.json # Custom 32-bit platform specification
│   └── rust-toolchain.toml    # Rust toolchain version pinning
├── src/         # C source implementation directory
│   ├── arch/    # Architecture specific setup code
│   │   └── x86/ # 32-bit initialization sequences
│   │       ├── boot/   # Assembly bootstrap logic and multiboot entry points
│   │       ├── cpu/    # Descriptor table logic and CPU contexts
│   │       └── memory/ # Memory layout managers, page routing, and allocation
│   ├── drivers/ # Physical & virtual hardware interface drivers
│   ├── kernel/  # Main execution pipelines and dual syscall routing routers
│   └── libkern/ # Kernel support libraries and internal utility logic
├── .clang-format   # Configuration of automatic C coding style format
├── .gitignore      # Build artifacts, and raw log filters
├── clean.py        # Automation script to safely delete build folders and files
├── grub.cfg        # GRUB setup template for the live ISO image
├── CONTRIBUTING.md # Guidelines for contributing and architectural rules
├── LICENSE.md      # Full legal text for the project's copyleft distribution license
├── linker.ld       # Defining memory section offsets and physical loading bounds
├── Makefile        # GNU Make multi stage instruction script
├── menu.lst        # Boot options template for direct GRUB configuration
├── muke.bat        # Compilation script for manual Windows builds
├── README.md       # Master repository documentation file
└── run.py          # Master orchestration script handling builds, assets, and QEMU setups
`````

---
## 🖥️ Emulating System Environment

To accurately reproduce the performance, bottlenecks, and hardware constraints of early-2010s netbooks (such as the classic ASUS Eee PC or Acer Aspire One), the virtual machine configuration is intentionally restricted. The profiles below are tailored for **QEMU** and **VirtualBox** to replicate the baseline environment of a first-generation Intel Atom device

| **Component** | **Specification** | **Emulation Target**                  |
| ------------- | ----------------- | ------------------------------------- |
| **CPU**       | 1 vCPU            | Mimics a single-core Intel Atom       |
| **Memory**    | 4GB RAM           | Standard capacity for Windows XP      |
| **Boot mode** | SeaBios           | Legacy boot environment               |
| **Storage**   | 8GB HDD           | Replicates early low-capacity storage |

---
## 🛠️ Prerequisites

- **C Compiler:** C23 compliant compiler (e.g., `gcc` or `clang`)
- **Rust Toolchain:** `rustc` and `cargo` v1.96+ (managed via `rustup` with bare-metal target support)
- **Assembler:** `nasm` (Netwide Assembler, used for low-level assembly routines)
- **Build System:** `make` (to parse the Makefile)
- **Toolchain:** `binutils` (for linking and binary manipulation)
- **ISO Utility:** `xorriso`, `grub-mkrescue` (essential for generating the `keios.iso` file)
- **Scripting Language:** `python 3.10+` (Required for running automation, build, and validation scripts)
- **Emulator:** `qemu` (specifically `qemu-system-i386`) or `VirtualBox` (Alternative for full virtual machine testing)

To build and run KeiOS, select the instructions matching the active development environment below

### 🐧 Linux (Ubuntu/Debian)

Install the required development tools, ISO utilities, Python environment, and emulators via the system package manager, then install Rust via `rustup`:

`````bash
sudo apt update
sudo apt install build-essential nasm xorriso mtools qemu-system-x86 grub-common grub2-common grub-pc-bin grub-efi-amd64-bin python3 curl
curl --proto '=https' --tlsv1.2 -sSf [https://sh.rustup.rs](https://sh.rustup.rs) | sh
`````

_(Note: `mtools` and `xorriso` are heavily used by tools like `grub-mkrescue` to build bootable ISO files. Install `grub2-common` if available on the distribution; otherwise, default to `grub-common`)_

### 🪟 Windows

Developing an operating system natively on Windows comes with significant hurdles. Before choosing a setup, review the downsides below

#### ⚠️ The Windows OS-Dev Downsides

- **The Binary Format Clash:** Windows compilers naturally output `PE/COFF` binaries (`.exe`, `.obj`). Kernel development strictly requires `ELF` format binaries. Compiling natively means a cross-compiler (`i686-elf-gcc`) must be set up manually, which is highly error-prone
- **The Bootloader Nightmare:** `grub-mkrescue` relies heavily on POSIX compliant paths, `bash`, and internal GNU utilities. Getting GRUB to successfully stitch together a bootable ISO natively on Windows is notoriously difficult
- **Line Ending Sabotage:** Git on Windows automatically converts line endings from LF `\n` to CRLF `\r\n`. This silent conversion can break linker scripts, assembly files, and Python build scripts without warning

#### Option 1: Windows Subsystem for Linux (WSL) - ⭐ **Highly Recommended**

This bypasses all Windows downsides by running a genuine Linux environment directly inside Windows

1. Open **PowerShell** as Administrator and install WSL:

`````powershell
wsl --install
`````

2. Restart the computer if prompted
3. Open the new Linux terminal (e.g., Ubuntu) and run the **Linux Prerequisites** command listed above

#### Option 2: Native Windows (MSYS2) - Advanced

If a native Windows environment is preferred, MSYS2 provides a reliable Unix like toolchain

1. Download and install [MSYS2](https://www.msys2.org/)
2. Open the **MSYS2 MINGW32** terminal window (this targets the 32-bit architecture KeiOS requires)
3. Install the compilation toolchain, assembler, Python, and ISO helpers:

`````bash
pacman -S mingw-w64-i686-gcc mingw-w64-i686-rust nasm make xorriso python3
`````

#### Option 3: Manual Addition - Final Resort

> Only use this option if getting WSL or MSYS2 running is not possible

1. Download the following packages, extract them, and organize them into a dedicated development folder (e.g., `C:\OperatingSystemTools`):

- **Assembler:** [NASM Windows Binaries](https://www.nasm.us/pub/nasm/releasebuilds/)
- **Cross-Compiler Alternative:** [i686-elf-tools (GitHub)](https://github.com/lordmilko/i686-elf-tools) _(Essential to output ELF format binaries from Windows)_
- **Rust Toolchain:** [Rustup for Windows](https://rustup.rs/)
- **ISO Creator:** [xorriso-exe-for-windows](https://github.com/PeyTy/xorriso-exe-for-windows)
- **Python:** [Python for Windows](https://www.python.org/downloads/windows/) _(Ensure "Add python.exe to PATH" is ticked during installation)_
- **Emulators:** [QEMU Windows Installer](https://qemu.weilnetz.de/w64/) and/or [VirtualBox Windows Installer](https://www.virtualbox.org/wiki/Downloads)
- **Bootloader Files:** [Legacy GRUB 0.97 i386-pc](https://alpha.gnu.org/gnu/grub/grub-0.97-i386-pc.tar.gz) _(Note: Use this specific version for manual ISO binding)._

1. **Environment Variables:** Add the paths of all extracted binaries to the system **PATH** variable
2. **GRUB Manual Setup:** Because `grub-mkrescue` is unavailable, the ISO structure must be formatted manually:

- Copy `iso9660_stage1_5` and `stage2` from the extracted GRUB files into the project's boot/root directory
- Rename `iso9660_stage1_5` to exactly `stage1`
- Use the downloaded Windows `xorriso` binary targeting this manual directory structure to build `keios.iso`

---
## 🏃 Build and Run

For **automatically building** and **running** execute the `run.py` python script:

`````bash
python run.py
`````

Also the `--help` flag can be passed to view available usage options:

`````bash
python run.py --help
`````

### 1. Compile the OS

Build the C kernel, compile the Rust crates, and generate the bootable `.iso` image using the provided Makefile

On **Linux** / **WSL**:

`````bash
make
`````

On **Native Windows** (**MSYS2 MINGW32**):

`````bash
mingw32-make
`````

For **Manual Addition (Windows)**: Run the provided batch script:
_(⚠️ Warning: this build system doesn't work, due to Rust integration)_
`````bash
muke.bat
`````

Pass `--help` flag to get all available usage options:

`````bash
# Use mingw32-make on Windows
make --help
`````

### 2. Launch in Emulator

KeiOS can be booted using one of two methods depending on the setup:

#### Method A: Direct Kernel Boot (No GRUB / No ISO Required) — ⚡ Fastest for Windows/MSYS2

If skipping the ISO build entirely is desired (or if on native Windows where `grub-mkrescue` isn't available), QEMU can be instructed to act as a Multiboot bootloader and load the kernel binary directly:

`````bash
qemu-system-i386 -kernel bin/keios.bin
`````

#### Method B: Standard ISO Boot (Requires GRUB/Xorriso)

If a full bootable CD image has been successfully generated and testing the actual bootloader configuration is needed:

`````bash
qemu-system-i386 keios.iso
`````

However, executing it via the `run.py` python script is recommended to get access to all features (such as 4GB memory, 8GB memory disk, network driver, etc.)

If executing the ISO manually is preferred, follow these instructions:

##### Build virtual disk

`````bash
qemu-img create -f qcow2 disk.qcow2 8G
`````

##### Execute the ISO

`````bash
qemu-system-i386 \
    -cpu n270 \
    -m 4G \
    -machine pc \
    -rtc base=localtime \
    -vga std \
    -hda disk.qcow2 \
    -net nic,model=rtl8139 -net user \
    -device intel-hda \
    -device hda-duplex \
    -d int,cpu_reset -D qemu.log \
    -cdrom keios.iso \
    -debugcon file:debug.log
`````

---
## 🔍 Troubleshooting & Common Pitfalls

OS development is a fragile process. If a build fails or QEMU refuses to launch properly, check these common issues first

### QEMU Instantly Reboots or Crashes (Triple Fault)

- **The Symptom:** QEMU launches, the window flashes or opens to a black screen, and CPU usage loops endlessly
- **The Cause:** The kernel hit a Triple Fault. This usually happens if the Global Descriptor Table (GDT) is misconfigured, an interrupt handler is missing, or the stack overflows before the kernel fully boots
- **The Fix:** Check the generated `qemu.log` and `debug.log` file in the project root. Look for the last executed instruction or a `[PANC]` message

_(Note: you can send `qemu.log` to your AI assistant with prompt to parse the file to find fault reason)_

### grub-mkrescue: error: xorriso not found (or mtools)

- **The Symptom:** Running `make` or `run.py` fails while trying to build `keios.iso`
- **The Cause:** `grub-mkrescue` acts as a compiler frontend and requires external backends to stitch the filesystem format together
- **The Fix:**
    - **Ubuntu/Debian/WSL:** Run `sudo apt install xorriso mtools`
    - **MSYS2:** Run `pacman -S xorriso`

### "Syntax Error" or "Invalid Command" in Linker/Assembly Scripts

- **The Symptom:** `nasm` or `ld` throws cryptic syntax errors on clean code paths
- **The Cause:** Line Ending Conversions. Git on Windows automatically converts files from LF (`\n`) to CRLF (`\r\n`) during checkouts. This breaks low-level assembly definitions and linker maps
- **The Fix:** Force Git to preserve line endings by running:

`````bash
git config --global core.autocrlf false
`````

Then re-clone the repository or fix individual scripts using `dos2unix`

### File Format Not Recognized During Linking

- **The Symptom:** The linker (`ld`) rejects compiled `.obj` or `.o` objects when linking `keios.bin`
- **The Cause:** A native Windows compiler target is being used. The host compiler outputs Windows PE/COFF architectures which cannot be mixed into flat binaries or standard ELF multiboot formats
- **The Fix:** Use `i686-elf-gcc` or target the MSYS2 MINGW32 environment strictly to output the correct target architecture formats

---
## 💬 Final Words

### SegFaultAt755

> **"If fighting is sure to result in victory, then you must fight!"**
> 
> _Sun Tzu said that, and I'd say he knows a little more about fighting than you do, pal, because he invented it, and then he perfected it so that no living man could best him in the ring of honor!_

### Siterfis

> **"With enough desire, you can do anything."**
> 
> _Even if it takes some time._

## 📄 License

This project is licensed under the **GNU General Public License v3.0 (GPLv3)**. See the `LICENSE` file for more details
