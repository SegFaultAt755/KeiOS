<div align="center">

![KEIOS-BANNER](./keios-banner.svg)

![Architecture: 32-bit i386](https://img.shields.io/badge/Arch-i386-blue?logo=intel&logoColor=white) ![License: GPLv3](https://img.shields.io/badge/License-GPLv3-green?logo=opensourceinitiative&logoColor=white) ![C23](https://img.shields.io/badge/C23-Standard-blue?logo=c) ![Rust](https://img.shields.io/badge/Rust-no__std-orange?logo=rust)

<!-- healthcheck:badges:start -->
![Clippy](https://img.shields.io/badge/Clippy-Not%20run-lightgrey) ![Build](https://img.shields.io/badge/Build-Not%20run-lightgrey)
<!-- healthcheck:badges:end -->

### An experimental 32-bit operating system built with a freestanding C23 kernel and Rust `no_std` drivers

---

# 🗺️ Table of Contents

</div>

<div align="center">

**Select a section link below to navigate directly to it**

| [🎯 Project Vision](#-project-vision) | [🏃 Build and Run](#-build-and-run) |
| :--- | :--- |
| [🚀 Key Features](#-key-features) | [⚙️ Compile](#1-compile-the-os) |
| [📂 Project Structure](#-project-structure) | [🚀 Launch](#2-launch-in-emulator) |
| [📦 Archive and Continuation](./docs/ARCHIVE.md) | [🔍 Troubleshooting](#-troubleshooting--common-pitfalls) |
| [🖥️ Emulating System Environment](#️-emulating-system-environment) | |
| [🛠️ Prerequisites](#️-prerequisites) | [📄 License](#-license) |
| [🐧 Linux (Ubuntu/Debian)](#-linux-ubuntudebian) | |
| [🪟 Windows (WSL2 / Docker)](#-windows-wsl2--docker) | |

</div>

---

<div align="center">

# 🎯 Project Vision

</div>

> **Archived experimental project:** KeiOS is an educational kernel experiment,
> not a production operating system. It has no practical deployment purpose or
> real-world use case, and should not be used for real workloads. See
> [the archive and continuation guide](./docs/ARCHIVE.md) for its status and
> where to resume development.

KeiOS is a 32-bit x86 operating-system project for experimenting with kernel
and driver design. It boots through GRUB/Multiboot into a freestanding C23
kernel, with a Rust `no_std` drivers library statically linked into the kernel.

The current system is a single privileged kernel with early subsystem
boundaries, not a microkernel. Process isolation, scheduling, general
executable loading, and user-space services remain future work. The current
boot flow and limitations are described in [docs/ARCHITECTURE.md](./docs/ARCHITECTURE.md)
and [docs/BOOT.md](./docs/BOOT.md).

---

<div align="center">

# 🚀 Key Features

</div>

* **Architecture:** 32-bit (`i386`) kernel booted by GRUB through Multiboot.
* **Hybrid implementation:** Freestanding C23 kernel with Rust `no_std` code
  for display, CPIO archive parsing, and IPC, connected through C ABI wrappers.
* **CPU and interrupts:** GDT/TSS and IDT setup, legacy PIC remapping, ISR/IRQ
  stubs, PIT timer, and an `int 0x80` syscall entry.
* **Memory:** Higher-half paging, a physical frame allocator using Multiboot
  memory ranges, and a 16-byte-aligned kernel heap.
* **Boot and devices:** ACPI discovery, CPIO initramfs parsing, PS/2 keyboard,
  and framebuffer output with VGA text fallback.
* **Shell:** A built-in kernel-mode polling shell. The initramfs callback can
  locate `bin/init.bin`, but its Ring 3 launch is disabled; there is no
  functioning process manager or isolated userspace.
* **Regression tests:** Python-driven tests cover the ISR ABI, CPIO parsing,
  heap allocation, and ACPI shutdown behavior.

KeiOS does not currently run Windows PE or Linux executables and does not
implement a dual syscall ABI.

---

<div align="center">

# 📂 Project Structure

</div>

```txt
KeiOS/
├── docs/                         # Archive handoff, architecture, boot, subsystem notes
│   ├── ARCHIVE.md                # Project status and continuation guide
│   └── subsystem/                # Memory, interrupts, process, and syscall docs
├── include/                      # C headers and shared configuration
│   ├── arch/x86/                 # x86 CPU, memory, interrupt, and syscall headers
│   ├── drivers/                  # Driver interfaces
│   ├── kernel/                   # Kernel service interfaces
│   └── libkern/                  # Kernel support-library interfaces
├── resources/                    # Source assets, including the bitmap font atlas
├── rootfs/                       # Files and directories included in the CPIO archive
├── rust/
│   └── crates/drivers/           # Rust display, CPIO, and IPC implementation
├── scripts/                      # Font generation, formatting, and Python requirements
├── src/
│   ├── arch/x86/                 # x86 boot, CPU/interrupt, and memory code
│   ├── drivers/                  # C hardware and terminal drivers
│   ├── kernel/                   # Boot orchestration, ACPI, shell, and services
│   └── libkern/                  # Kernel support-library implementation
├── tests/                        # Python regression tests and native harnesses
├── userspace/
│   ├── init/                     # Early init program source
│   └── stdlib/                   # Early userspace support code
├── AGENTS.md                     # Repository-specific agent guidance
├── build.py                      # Build wrapper
├── clean.py                      # Build cleanup helper
├── grub.cfg                      # GRUB Multiboot configuration
├── healthcheck.py                # Rust lint and build health checks
├── linker.ld                     # Kernel linker script
├── Makefile                      # Kernel, initramfs, ISO, and test targets
└── run.py                        # QEMU launcher
```

`bin/`, `keios/`, `keios.elf`, `initramfs.cpio`, and `keios.iso` are generated
build outputs; `rootfs/` and `userspace/` contain source inputs.

---

<div align="center">

# 🖥️ Emulating System Environment

</div>

`run.py` boots the ISO in QEMU using a PC machine profile. Its defaults emulate
an Intel N270 CPU, allocate 4 GiB of RAM, and create an 8 GiB QCOW2 disk as
needed. These are development defaults, not hardware requirements. The script
also configures the emulated network/audio devices and writes `qemu.log` and
`debug.log` in the repository root.

---

<div align="center">

# 🛠️ Prerequisites

</div>

* **C toolchain:** An `i686-elf` GCC/binutils cross-compiler, or a C23-capable
  GCC with 32-bit multilib support.
* **Rust:** Rust nightly with the `rust-src` component; Clippy is needed for
  the health check.
* **Build tools:** GNU Make, NASM, and CPIO.
* **ISO tools:** GRUB utilities (`grub-mkrescue`), `xorriso`, and `mtools`.
* **Python:** Python 3 and Pillow (listed in `scripts/requirements.txt`).
* **Emulator:** QEMU with `qemu-system-i386` and `qemu-img`.

For Linux and other supported environments, create and activate a Python
virtual environment, then install the project dependencies from the
requirements file:

```bash
python3 -m venv .venv
source .venv/bin/activate
python3 -m pip install -r scripts/requirements.txt
```

The default Makefile toolchain prefix is `i686-elf-`. With a host GCC that
supports C23 and 32-bit output, override the compiler settings, for example:

```bash
make TOOLCHAIN_PREFIX= CC=gcc LD=ld
```

To install the remaining build tools and Python dependency on Ubuntu/Debian:

<div align="center">

## 🐧 Linux (Ubuntu/Debian)

</div>

```bash
sudo apt update
sudo apt install build-essential gcc-multilib nasm binutils make cpio \
  grub-common grub-pc-bin xorriso mtools python3 python3-pip python3-venv \
  qemu-system-x86
```

Install Rust nightly and its source component with `rustup`:

```bash
rustup toolchain install nightly --component rust-src --component clippy
rustup default nightly
```

For Windows, use a Linux development environment:

<div align="center">

## 🪟 Windows (WSL2 / Docker)

</div>

Install WSL2 and an Ubuntu distribution, then follow the Linux setup above.
Docker users can build inside a Linux container with the project directory
mounted into it. QEMU graphical output depends on the host's display support;
use `--headless` when a GUI is unavailable.

---

<div align="center">

# 🏃 Build and Run

</div>

<div align="center">

## 1. Compile the OS

</div>

Build the kernel, CPIO initramfs, and bootable ISO:

```bash
make
```

To clean and rebuild:

```bash
python3 build.py --clean-build
```

To remove generated build outputs:

```bash
make clean
```

Run the regression tests:

```bash
make test
```

Rust formatting and correctness lint checks:

```bash
cd rust && cargo fmt --all -- --check
cd rust && cargo clippy -- -A clippy::all -D clippy::correctness
```

<div align="center">

## 2. Launch in Emulator

</div>

Build and run the ISO in QEMU with the default configuration:

```bash
python3 run.py
```

Run without a graphical display, or skip the build when the ISO already exists:

```bash
python3 run.py --headless
python3 run.py --skip-build
```

Use `python3 run.py --help` to see additional options, including the QEMU CPU,
memory, disk, and build-target settings. To build without starting QEMU, use
`python3 build.py`.

---

<div align="center">

# 🔍 Troubleshooting & Common Pitfalls

</div>

<div align="center">

## ISO creation fails because `xorriso`, `mtools`, or GRUB is missing

</div>

`grub-mkrescue` depends on the GRUB, `xorriso`, and `mtools` utilities. Install
the ISO tools listed in the prerequisites, then rerun `make`.

<div align="center">

## QEMU hangs, resets, or shows a blank screen

</div>

Check `debug.log` for the last kernel messages and `qemu.log` for CPU/interrupt
state. The kernel's boot and subsystem notes are in [docs/BOOT.md](./docs/BOOT.md)
and [docs/ARCHITECTURE.md](./docs/ARCHITECTURE.md).

---

<div align="center">

# 📄 License

</div>

This project is licensed under the **GNU General Public License v3.0
(GPLv3)**. See [LICENSE.md](./LICENSE.md) for details.
