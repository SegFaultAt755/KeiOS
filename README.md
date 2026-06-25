# KeiOS

> A 32-bit (`i386`) lab operating system built to experiment with cross-platform executable support via dual syscall tables.

*⚠️ Note: KeiOS is currently in active development. (Current focus: Implementing the Global Descriptor Table / GDT).*

---

## 🎯 Project Vision

The primary goal of KeiOS is to serve as an experimental lab environment. Its standout feature is the planned implementation of **two distinct syscall tables**, allowing the OS to natively handle and execute both **Windows** and **Linux** binaries within a minimal custom kernel and user space.

---

## 🚀 Key Features

* **Architecture:** 32-bit x86 (`i386`) | Microkernel architecture.
* **Dual Syscall Interface:** Designed to route system calls for both Windows and Linux executables.
* **Modular Design:** Clean separation of bootloader, kernel, and user space code.
* **Minimal Tooling:** Straightforward build process relying on standard C compilers and Makefiles.

---

## 🗺️ Repository Structure

The codebase is strictly organized by system layer:

```text
└── src/
    ├── boot/       # Bootloader and early initialization code
    ├── kernel/     # Core kernel logic, memory management, and syscall routing
    └── user/       # User space applications, libraries, and test executables
├── Makefile        # Build configuration
└── README.md       # Project documentation

```

---

## 🛠️ Prerequisites

* **Compiler:** A C23-compliant compiler (e.g., `gcc` or `clang`)
* **Assembler:** `nasm` (Netwide Assembler, used for low-level assembly routines)
* **Build System:** `make` (to parse your Makefile)
* **Toolchain:** `binutils` (for linking and binary manipulation)
* **ISO Utility:** `xorriso` or `grub-mkrescue` (essential for generating the `keios.iso` file)
* **Emulator:** `qemu` (specifically `qemu-system-i386`)

To build and run KeiOS, select the instructions matching your development environment below.

### 🐧 Linux (Ubuntu/Debian)

Install the required development tools, ISO utilities, and the emulator via your package manager:

```bash
sudo apt update
sudo apt install build-essential nasm xorriso mtools qemu-system-x86

```

*(Note: `mtools` and `xorriso` are heavily used by dependency tools like `grub-mkrescue` to build actual ISO files).*

### 🪟 Windows

#### Option 1: Windows Subsystem for Linux (WSL) — *Recommended*
1. Open **PowerShell** as Administrator and install WSL:
```powershell
wsl --install

```

2. Restart your computer if prompted.
3. Open your new Linux terminal (e.g., Ubuntu) and follow the **Linux Prerequisites** instructions listed above.

#### Option 2: Native Windows (MSYS2)

1. Download and install [MSYS2](https://www.msys2.org/).
2. Open the **MSYS2 MINGW32** terminal (required since KeiOS targets a 32-bit architecture) and install the toolchain:
```bash
pacman -S mingw-w64-i686-gcc make nasm

```

3. Download and install [QEMU for Windows](https://www.google.com/search?q=https://www.qemu.org/download/%23windows).
4. **Crucial:** Make sure to check the box to add QEMU to your system's environment variables (`PATH`) during the installation process.

---

## 🏃 Build and Run

### 1. Compile the OS

Build the kernel and generate the bootable `.iso` image using the provided Makefile:

```bash
make

```

### 2. Launch in Emulator

Boot the compiled image using QEMU:

```bash
qemu-system-i386 keios.iso

```

---

## 🗺️ Roadmap

* [x] Initial project scaffolding and minimal build system
* [x] Basic bootloader configuration
* [ ] **In Progress:** Implementing the Global Descriptor Table (GDT)
* [ ] Interrupt Descriptor Table (IDT) and ISRs
* [ ] Memory Management (Paging)
* [ ] Linux Syscall Table implementation
* [ ] Windows Syscall Table implementation
* [ ] Basic User Space Shell

---

## 📄 License

This project is licensed under the **GNU General Public License v3.0 (GPLv3)**. See the `LICENSE` file for more details.