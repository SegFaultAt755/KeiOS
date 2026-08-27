<div align="center">

![KEIOS-BANNER](./keios-banner.svg)

![Architecture: 32-bit i386](https://img.shields.io/badge/Arch-i386-blue?logo=intel&logoColor=white) ![License: GPLv3](https://img.shields.io/badge/License-GPLv3-green?logo=opensourceinitiative&logoColor=white) ![Rust](https://img.shields.io/badge/Rust-1.95-orange?logo=rust) ![C23](https://img.shields.io/badge/C23-Standard-blue?logo=c)

![Build](https://img.shields.io/badge/Build-Passing-green) ![Runtime](https://img.shields.io/badge/Runtime-Success-green)

### 32-bit lab operating system built to experiment with cross-platform executable support via dual syscall tables, powered by a modern C23 and Rust architecture

---

# 🗺️ Table of Contents

</div>

<div align="center">

**Select a section link below to navigate directly to it**

| [🎯 Project Vision](#-project-vision) | [🏃 Build and Run](#-build-and-run) |
| :--- | :--- |
| [🚀 Key Features](#-key-features) | [⚙️ Compile](#1-compile-the-os) |
| [📂 Project Structure](#-project-structure) | [🚀 Launch](#2-launch-in-emulator) |
| [🖥️ Emulating System Environment](#%EF%B8%8F-emulating-system-environment) | [🔍 Troubleshooting & Common Pitfalls](#-troubleshooting--common-pitfalls) |
| [🛠️ Prerequisites](#%EF%B8%8F-prerequisites) | [💬 Final Words](#-final-words) |
| [🐧 Linux (Ubuntu/Debian)](#-linux-ubuntudebian) | [📄 License](#-license) |
| [🪟 Windows (WSL2 / Docker)](#-windows-wsl2--docker) | |

</div>

---

<div align="center">

# 🎯 Project Vision

</div>

The primary goal of KeiOS is to serve as an experimental lab environment. Its standout feature is the planned implementation of **two distinct syscall tables**, allowing the OS to natively handle and execute both **Windows** and **Linux** binaries within a minimal custom kernel and user space

---

<div align="center">

# 🚀 Key Features

</div>

* **Hybrid Core Architecture:** Built using modern C23 and Rust (v1.96) with seamless FFI integration for memory-safe driver and kernel component development
* **Architecture:** 32-bit (`i386`) operating on a microkernel architecture
* **Dual Syscall Interface:** Purpose-built routing system to process system calls from both Windows and Linux executables natively
* **Modular Design:** Clean structural separation between the bootloader, kernel, drivers, and user space code

---

<div align="center">

# 📂 Project Structure

</div>

```txt
KeiOS/
├── bin/                                # Compiled binary artifacts
├── include/                            # Main header directory
│   ├── arch/                           # Architecture-specific header definitions
│   │   └── x86/                        # 32-bit control registers, segmentations, and offsets
│   ├── drivers/                        # Peripheral configurations (network, audio, display)
│   ├── kernel/                         # Microkernel core system
│   ├── libkern/                        # Mini C standard library implementations for kernel space
│   └── config.h                        # Static build configuration toggles
├── rust/                               # Rust workspace for memory-safe kernel components and drivers
│   ├── .cargo/                         # Cargo build configuration and target flags
│   ├── crates/                         # Modular Rust crates (e.g., drivers, kernel, syslibs)
│   ├── Cargo.toml                      # Workspace root dependency manager
│   ├── i686-unknown-none.json          # Custom 32-bit bare-metal platform specification
│   └── rust-toolchain.toml             # Rust toolchain version pinning
├── src/                                # C source implementation directory
│   ├── arch/                           # Architecture-specific setup code
│   │   └── x86/                        # 32-bit initialization sequences
│   │       ├── boot/                   # Assembly bootstrap logic and multiboot entry points
│   │       ├── cpu/                    # Descriptor table logic and CPU contexts
│   │       └── memory/                 # Memory layout managers, page routing, and allocation
│   ├── drivers/                        # Physical & virtual hardware interface drivers
│   ├── kernel/                         # Main execution pipelines and dual syscall routing routers
│   └── libkern/                        # Kernel support libraries and internal utility logic
├── .clang-format                       # Configuration for automatic C coding style formatting
├── .gitignore                          # Build artifacts and raw log filters
├── clean.py                            # Automation script to safely delete build folders and files
├── grub.cfg                            # GRUB setup template for the live ISO image
├── CONTRIBUTING.md                     # Guidelines for contributing and architectural rules
├── LICENSE.md                          # Full legal text for the project's copyleft distribution license
├── linker.ld                           # Defining memory section offsets and physical loading bounds
├── Makefile                            # GNU Make build script
├── README.md                           # Master repository documentation file
└── run.py                              # Master orchestration script handling builds, assets, and QEMU setups
```

---

<div align="center">

# 🖥️ Emulating System Environment

</div>

To accurately reproduce the performance, bottlenecks, and hardware constraints of early-2010s netbooks (such as the classic ASUS Eee PC or Acer Aspire One), the virtual machine configuration is intentionally restricted. The profiles below are tailored for **QEMU** and **VirtualBox** to replicate the baseline environment of a first-generation Intel Atom device:

<div align="center">

| **Component** | **Specification** | **Emulation Target**                  |
| ------------- | ----------------- | ------------------------------------- |
| **CPU**       | 1 vCPU            | Mimics a single-core Intel Atom       |
| **Memory**    | 4GB RAM           | Standard capacity for Windows XP      |
| **Boot mode** | SeaBios           | Legacy boot environment               |
| **Storage**   | 8GB HDD           | Replicates early low-capacity storage |

</div>

---

<div align="center">

# 🛠️ Prerequisites

</div>

* **C Compiler:** C23 compliant cross-compiler (`i686-elf-gcc` or standard Linux `gcc` with `-m32` support)
* **Rust Toolchain:** `rustc` and `cargo` v1.96+ (managed via `rustup` with bare-metal target support)
* **Assembler:** `nasm` (Netwide Assembler, used for low-level assembly routines)
* **Build System:** `make` (GNU Make)
* **Toolchain:** `binutils` (for linking and binary manipulation)
* **ISO Utility:** `xorriso`, `mtools`, and `grub-mkrescue` (essential for generating the `keios.iso` file)
* **Scripting Language:** `python 3.10+` (Required for running automation, build, and validation scripts)
* **Emulator:** `qemu` (specifically `qemu-system-i386`) or `VirtualBox`

To build and run KeiOS, select the instructions matching your active development environment below:

<div align="center">

## 🐧 Linux (Ubuntu/Debian)

</div>

Install the required development tools, ISO utilities, Python environment, and emulators via the system package manager, then install Rust via `rustup`:

```bash
sudo apt update
sudo apt install build-essential nasm xorriso mtools qemu-system-x86 grub-common grub2-common grub-pc-bin grub-efi-amd64-bin python3 curl
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
```

*(Note: `mtools` and `xorriso` are heavily used by tools like `grub-mkrescue` to build bootable ISO files. Install `grub2-common` if available on your distribution, otherwise, default to `grub-common`)*

<div align="center">

## 🪟 Windows (WSL2 / Docker)

</div>

> [!IMPORTANT]
> **Why Native Windows Build Support Was Stripped:** Attempting to compile the OS natively on Windows (using Command Prompt, PowerShell, and standard MSYS2/MinGW) introduces severe, deal-breaking friction:
> * **The Binary Format Clash:** Windows linkers natively output `PE/COFF` formats (`.exe`). An OS kernel requires raw bare-metal `ELF32` binaries. Forcing Windows linkers to emulate Linux targets or relying on custom `objcopy` hacks causes constant build failures
> * **The Bootloader Nightmare:** Generating a bootable image relies on `grub-mkrescue`, which deeply depends on POSIX paths, Unix shell scripting, and tools like `xorriso` and `mtools` that lack stable native Windows ports
> * **Shell & Path Incompatibilities:** Supporting Windows `cmd.exe` syntax (`if not exist`, backslashes `\`, null devices `nul`) alongside POSIX Bash syntax makes the `Makefile` unmaintainable and fragile for contributors
> 
> 
> **For a frictionless experience, Windows users MUST use one of the two standard POSIX methods below**

<div align="center">

### Option 1: Windows Subsystem for Linux (WSL2) ⭐ **Recommended**

</div>

WSL2 runs a genuine, lightning-fast Linux kernel directly inside Windows without the overhead of a traditional VM

1. Open **PowerShell** as Administrator and install WSL:

```powershell
wsl --install
```

2. Restart your computer if prompted
3. Open your new Linux terminal (e.g., Ubuntu) from the Start Menu
4. Run the **Linux (Ubuntu/Debian)** installation commands listed in the section above. You can now build and run QEMU directly inside WSL

<div align="center">

### Option 2: Docker Container (Zero-Install Build)

</div>

If you have Docker Desktop installed on Windows and don't want to install build tools locally, you can compile KeiOS inside an isolated container:

1. Launch a temporary Ubuntu build container mounted to your project folder:

```cmd
docker run --rm -it -v "%cd%":/root/KeiOS -w /root/KeiOS ubuntu:24.04 bash
```

2. Inside the container, run the Linux package install command and execute `make`

---

<div align="center">

# 🏃 Build and Run

</div>

For **automatically building** and **running** the project, execute the `run.py` Python script:

```bash
python run.py
```

You can also pass the `--help` flag to view available usage options:

```bash
python run.py --help
```

<div align="center">

## 1. Compile the OS

</div>

Build the C kernel, compile the Rust crates, and generate the bootable `.iso` image using the standard GNU Makefile:

```bash
make
```

To clean up build artifacts and temporary files:

```bash
python clean.py
```

If you need to clean up only build artifacts:

```bash
make clean
```

<div align="center">

## 2. Launch in Emulator

</div>

KeiOS can be booted using one of two methods depending on your workflow:

<div align="center">

### Method A: Standard ISO Boot (Recommended)

</div>

If a full bootable CD image (`keios.iso`) has been successfully generated via `make`, launch it directly in QEMU:

```bash
qemu-system-i386 keios.iso
```

> **Note:** Executing it via the `run.py` Python script is strongly recommended to automatically configure all hardware flags (4GB memory, 8GB storage disk, RTL8139 network driver, and audio emulation)

If executing the ISO manually with full hardware emulation is preferred, follow these instructions:

<div align="center">

#### 1. Build the virtual storage disk:

</div>

```bash
qemu-img create -f qcow2 disk.qcow2 8G
```

<div align="center">

#### 2. Execute the ISO with netbook hardware emulation:

</div>

```bash
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
```

<div align="center">

### Method B: Direct Kernel Boot (No ISO Required)

</div>

If you are debugging kernel internals and want to skip GRUB ISO generation entirely, QEMU can act as a Multiboot bootloader and load the raw kernel binary directly:

```bash
qemu-system-i386 -kernel bin/keios.elf
```

*(Note: Some advanced GRUB-dependent features or initial ramdisks will be bypassed in this mode)*

---

<div align="center">

# 🔍 Troubleshooting & Common Pitfalls

</div>

OS development is a fragile process. If a build fails or QEMU refuses to launch properly, check these common issues first:

<div align="center">

## QEMU Instantly Reboots or Crashes (Triple Fault)

</div>

* **The Symptom:** QEMU launches, the window flashes or opens to a black screen, and CPU usage loops endlessly
* **The Cause:** The kernel hit a Triple Fault. This usually happens if the Global Descriptor Table (GDT) is misconfigured, an interrupt handler is missing, or the stack overflows before the kernel fully boots
* **The Fix:** Check the generated `qemu.log` and `debug.log` files in the project root. Look for the last executed instruction or a `[PANC]` message

*(Note: Send `qemu.log` file to AI and prompt to parse it, it will **'maybe'** give you the right answer what crashed)*

<div align="center">

## `grub-mkrescue: error: xorriso not found` (or `mtools`)

</div>

* **The Symptom:** Running `make` or `run.py` fails while trying to generate `keios.iso`
* **The Cause:** `grub-mkrescue` acts as a frontend and requires external backends to stitch the ISO filesystem together
* **The Fix:** Ensure you have installed the filesystem utilities:

```bash
sudo apt install xorriso mtools
```

<div align="center">

## "Syntax Error" or "Invalid Command" in Linker/Assembly Scripts

</div>

* **The Symptom:** `nasm` or `ld` throws cryptic syntax errors on clean code paths
* **The Cause:** **Line Ending Conversions** Git on Windows automatically converts files from LF (`\n`) to CRLF (`\r\n`) during checkouts. This breaks low-level assembly definitions, shell scripts, and linker maps
* **The Fix:** Force Git to preserve POSIX line endings by running:

```bash
git config --global core.autocrlf false
```

Then re-clone the repository or fix individual files using `dos2unix`

---

<div align="center">

# 💬 Final Words

## SegFaultAt755

> **"If fighting is sure to result in victory, then you must fight!"**
> *Sun Tzu said that, and I'd say he knows a little more about fighting than you do, pal, because he invented it, and then he perfected it so that no living man could best him in the ring of honor!*

## Siterfis

> **"With enough desire, you can do anything."**
> *Even if it takes some time.*

</div>

---

<div align="center">

# 📄 License

</div>

This project is licensed under the **GNU General Public License v3.0 (GPLv3)**. See the `LICENSE.md` file for more details
