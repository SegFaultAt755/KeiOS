# KeiOS

> 32-bit lab operating system built to experiment with cross-platform executable support via dual syscall tables.

*(⚠️ Note: KeiOS is currently in active development...)*

---

## 🎯 Project Vision

The primary goal of KeiOS is to serve as an experimental lab environment. Its standout feature is the planned implementation of **two distinct syscall tables**, allowing the OS to natively handle and execute both **Windows** and **Linux** binaries within a minimal custom kernel and user space.

---

## 🚀 Key Features

* **Architecture:** 32-bit x86 (`i386`) operating on a microkernel architecture.
* **Dual Syscall Interface:** Purpose-built routing system to process system calls from both Windows and Linux executables natively.
* **Modular Design:** Clean structural separation between the bootloader, kernel, and user space code.

---

## 🖥️ Emulating System Environment

To accurately reproduce the performance, bottlenecks, and hardware constraints of late-2000s netbooks (such as the classic ASUS Eee PC or Acer Aspire One), the virtual machine configuration is intentionally restricted. The profiles below are tailored for **QEMU** and **VirtualBox** to replicate the baseline environment of a first-generation Intel Atom device.

| Component     | Specification | Notes / Emulation Target                                        |
| ------------- | ------------- | --------------------------------------------------------------- |
| **CPU**       | 1 vCPU        | Mimics a single-core Intel Atom                                 |
| **Memory**    | 1GB RAM       | Standard capacity for Windows XP                                |
| **Boot mode** | BIOS          | Legacy boot environment                                         |
| **Storage**   | 8GB HDD       | Replicates early low-capacity storage or slow mechanical drives |

---

## 🛠️ Prerequisites

* **C Compiler:** A C23-compliant compiler (e.g., `gcc` or `clang`)
* **Assembler:** `nasm` (Netwide Assembler, used for low-level assembly routines)
* **Build System:** `make` (to parse the Makefile)
* **Toolchain:** `binutils` (for linking and binary manipulation)
* **ISO Utility:** `xorriso`, `grub-mkrescue` (essential for generating the `keios.iso` file)
* **Scripting Language:** `python 3.13` (Required for running automation, build, and validation scripts)
* **Emulator:** `qemu` (specifically `qemu-system-i386`) or `VirtualBox` (Alternative for full virtual machine testing)

To build and run KeiOS, select the instructions matching your development environment below.

### 🐧 Linux (Ubuntu/Debian)

Install the required development tools, ISO utilities, Python environment, and emulators via your package manager:

```bash
sudo apt update
sudo apt install build-essential nasm xorriso mtools qemu-system-x86 grub-common grub2-common grub-pc-bin grub-efi-amd64-bin python3
```

*(Note: `mtools` and `xorriso` are heavily used by tools like `grub-mkrescue` to build bootable ISO files. Install `grub2-common` if available on your distribution; otherwise, default to `grub-common`).*

---

### 🪟 Windows

Developing an operating system natively on Windows comes with significant hurdles. Before choosing your setup, review the downsides below.

#### ⚠️ The Windows OS-Dev Downsides

* **The Binary Format Clash:** Windows compilers naturally output `PE/COFF` binaries (`.exe`, `.obj`). Kernel development strictly requires `ELF` format binaries. Compiling natively means you must manually set up a cross-compiler (`i686-elf-gcc`), which is highly error-prone.
* **The Bootloader Nightmare:** `grub-mkrescue` relies heavily on POSIX-compliant paths, `bash`, and internal GNU utilities. Getting GRUB to successfully stitch together a bootable ISO natively on Windows is notoriously difficult.
* **Line Ending Sabotage:** Git on Windows automatically converts line endings from LF (`\n`) to CRLF (`\r\n`). This silent conversion can break your linker scripts, assembly files, and Python build scripts without warning.

---

#### Option 1: Windows Subsystem for Linux (WSL) - ⭐ *Highly Recommended*

This bypasses all Windows downsides by running a genuine Linux environment directly inside Windows.

1. Open **PowerShell** as Administrator and install WSL:

```powershell
wsl --install
```

2. Restart your computer if prompted.
3. Open your new Linux terminal (e.g., Ubuntu) and run the **Linux Prerequisites** command listed above.

---

#### Option 2: Native Windows (MSYS2) - Advanced

If you prefer a native Windows environment, MSYS2 provides a reliable Unix-like toolchain.

1. Download and install [MSYS2](https://www.msys2.org/).
2. Open the **MSYS2 MINGW32** terminal window (this targets the 32-bit architecture KeiOS requires).
3. Install the compilation toolchain, assembler, Python, and ISO helpers:

```bash
pacman -S mingw-w64-i686-gcc mingw-w64-i686-make nasm xorriso python3
```

---

#### Option 3: Manual Addition - Final Resort

> Only use this option if you cannot get WSL or MSYS2 running.

1. Download the following packages, extract them, and organize them into a dedicated development folder (e.g., `C:\OperatingSystemTools`):

* **Assembler:** [NASM Windows Binaries](https://www.nasm.us/pub/nasm/releasebuilds/)
* **Cross-Compiler Alternative:** [i686-elf-tools (GitHub)](https://github.com/lordmilko/i686-elf-tools) *(Essential to output ELF format binaries from Windows)*
* **ISO Creator:** [xorriso-exe-for-windows](https://github.com/PeyTy/xorriso-exe-for-windows)
* **Python:** [Python for Windows](https://www.python.org/downloads/windows/) *(Ensure "Add python.exe to PATH" is ticked during installation)*
* **Emulators:** [QEMU Windows Installer](https://qemu.weilnetz.de/w64/) and/or [VirtualBox Windows Installer](https://www.virtualbox.org/wiki/Downloads)
* **Bootloader Files:** [Legacy GRUB 0.97 i386-pc](https://alpha.gnu.org/gnu/grub/grub-0.97-i386-pc.tar.gz) *(Note: Use this specific version for manual ISO binding).*

2. **Environment Variables:** Add the paths of all extracted binaries to your system **PATH** variable.
3. **GRUB Manual Setup:** Because you don't have `grub-mkrescue`, you must manually format the ISO structure:

* Copy `iso9660_stage1_5` and `stage2` from the extracted GRUB files into the project's boot/root directory.
* Rename `iso9660_stage1_5` to exactly `stage1`.
* Use your downloaded Windows `xorriso` binary targeting this manual directory structure to build `keios.iso`.

---

## 🏃 Build and Run

### 1. Compile the OS

Build the kernel and generate the bootable `.iso` image using the provided Makefile.

On **Linux** / **WSL**:

```bash
make
```

On **Native Windows** (**MSYS2 MINGW32**):

```bash
mingw32-make
```

For **Manual Addition (Windows)**: Run the provided batch script:

```cmd
muke.bat
```

### 2. Launch in Emulator

You can boot KeiOS using one of two methods depending on your setup:

#### Method A: Direct Kernel Boot (No GRUB / No ISO Required) — ⚡ Fastest for Windows/MSYS2

If you want to skip building an ISO entirely (or if you are on native Windows where `grub-mkrescue` isn't available), you can instruct QEMU to act as a Multiboot bootloader and load your kernel binary directly:

```bash
qemu-system-i386 -kernel bin/keios.bin
```

*(Note: Replace `bin/keios.bin` with the actual path to your compiled kernel executable file generated by your Makefile).*

#### Method B: Standard ISO Boot (Requires GRUB/Xorriso)

If you have successfully generated a full bootable CD image and want to test the actual bootloader configuration:

```bash
python3 run.py
```

---

## 💬 Final Words

### SegFaultAt755

**"If fighting is sure to result in victory, then you must fight!"**
*Sun Tzu said that, and I'd say he knows a little more about fighting than you do, pal, because he invented it, and then he perfected it so that no living man could best him in the ring of honor!*

### Siterfis

**"With enough desire, you can do anything."**
*Even if it takes some time.*

---

## 📄 License

This project is licensed under the **GNU General Public License v3.0 (GPLv3)**. See the `LICENSE` file for more details.
