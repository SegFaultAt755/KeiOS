<div align="center">

<img src="./keios-banner.svg" alt="KeiOS" width="720">

<br>

![i386](https://img.shields.io/badge/Architecture-i386-2563eb?logo=intel&logoColor=white)
![C23](https://img.shields.io/badge/Kernel-C23-0f766e)
![Rust nightly](https://img.shields.io/badge/Rust-nightly-d97706?logo=rust&logoColor=white)
![GPLv3](https://img.shields.io/badge/License-GPLv3-15803d)

[![GitHub stars](https://img.shields.io/github/stars/SegFaultAt755/KeiOS?style=for-the-badge&logo=github)](https://github.com/SegFaultAt755/KeiOS/stargazers)
[![GitHub forks](https://img.shields.io/github/forks/SegFaultAt755/KeiOS?style=for-the-badge&logo=github)](https://github.com/SegFaultAt755/KeiOS/network/members)
[![Open issues](https://img.shields.io/github/issues/SegFaultAt755/KeiOS?style=for-the-badge&logo=github)](https://github.com/SegFaultAt755/KeiOS/issues)
[![Last commit](https://img.shields.io/github/last-commit/SegFaultAt755/KeiOS?style=for-the-badge&logo=github)](https://github.com/SegFaultAt755/KeiOS/commits)

<!-- healthcheck:badges:start -->
![Clippy](https://img.shields.io/badge/Clippy-Passing-brightgreen) ![Build](https://img.shields.io/badge/Build-Passing-brightgreen)
<!-- healthcheck:badges:end -->

### An experimental 32-bit operating system, built to explore kernel design.

KeiOS is a small learning project for exploring how an operating system boots
and how kernel components fit together. You can build it into a bootable ISO
and try it in QEMU without installing it on your computer.

</div>

---

<div align="center">

## 📖 Table of contents

</div>

- [Project at a glance](#project-at-a-glance)
- [Current capabilities](#-current-capabilities)
- [Getting started](#-getting-started)
  - [Install prerequisites](#1-install-prerequisites)
  - [Build the ISO](#2-build-the-iso)
  - [Run the regression tests](#3-run-the-regression-tests)
  - [Run in QEMU](#4-run-in-qemu)
- [Scripts](#-scripts)
- [Project structure](#-project-structure)
- [Documentation](#-documentation)

---

<div align="center">

## Project at a glance

</div>

KeiOS is an experimental **i386 operating system** combining a freestanding
C23 kernel with a Rust `no_std` drivers library. Today, it boots through GRUB,
initializes core CPU and memory facilities, reads a CPIO initramfs, brings up
basic hardware, and presents an interactive kernel shell.

If you are new to operating systems, a good first step is to build and run the
ISO in QEMU, then explore the boot flow and architecture guides below. KeiOS is
actively experimental: expect incomplete features, rough edges, and ongoing changes.

> [!NOTE]
> KeiOS is an early-stage research project, not a general-purpose operating
> system. Process isolation, general ELF execution, and Windows or Linux binary
> compatibility are future goals—not current features.

<div align="center">

| **Target** | **Kernel** | **Boot** | **License** |
|:---:|:---:|:---:|:---:|
| 32-bit x86 | C23 + Rust | GRUB / Multiboot | GPL-3.0 |

</div>

---

<div align="center">

## ✨ Current capabilities

</div>

| Area | Available today |
| --- | --- |
| **Boot** | GRUB Multiboot boot into a 32-bit higher-half kernel |
| **CPU** | Early GDT/IDT setup, PIC remapping, CPU feature checks, and PIT timer |
| **Memory** | Experimental paging, physical frame bitmap, and kernel heap |
| **Input & display** | PS/2 keyboard, VGA text mode, and an RGB framebuffer path with VGA fallback |
| **Initramfs** | CPIO archive parsing from a GRUB-loaded module |
| **Kernel services** | A syscall dispatcher and bounded in-kernel IPC queues |
| **Shell** | Ring 0 command shell: `help`, `clear`, `echo`, `ver`, `uptime`, `meminfo`, `color`, `datetime`, `reboot`, `shutdown`, `halt` |

The shell currently runs in kernel mode. There is no working scheduler or
general process loader, and the bundled userspace init program is not started
by the normal boot path. KeiOS does **not** currently run Windows PE or Linux
executables. Existing low-level facilities are experimental and should not be
considered production-ready.

> [!WARNING]
> The current boot flow is still kernel-only. `userspace/`, `rootfs/`, and the
> associated initramfs logic are build-time scaffolding for future execution
> paths; the default boot path does not start a user process or scheduler.

---

<div align="center">

## 🚀 Getting started

</div>

The quickest route is to install the build tools, build the ISO, and launch it
in QEMU. Run the following commands from the KeiOS repository directory.

### Install prerequisites

The supported build and run environment is Linux or Linux under WSL2. On
Debian or Ubuntu, install the host tools:

```sh
sudo apt update
sudo apt install build-essential nasm binutils cpio xorriso mtools \
  grub-common grub-pc-bin qemu-system-x86 qemu-utils python3
```

KeiOS also needs an **i686-elf cross-compiler** (`i686-elf-gcc` and
`i686-elf-ld`). The `binutils` package above provides host utilities; it does
not necessarily include this cross-compiler. Install an i686-elf toolchain
using your distribution's packages or follow the
[OSDev cross-compiler guide](https://wiki.osdev.org/GCC_Cross-Compiler), and
ensure its binaries are available in your `PATH`.

Install [Rust using rustup](https://rustup.rs), then install the project's
nightly toolchain and source component:

```sh
rustup toolchain install nightly
rustup component add rust-src --toolchain nightly
```

The Rust project pins nightly in `rust/rust-toolchain.toml` and builds the
`core`, `alloc`, and compiler-builtins libraries from source. The `rust-src`
component is required for that build.

### Build the ISO

Build the kernel, initramfs, and bootable ISO from the repository root:

```sh
make
```

The default Make target creates `keios.elf`, `initramfs.cpio`, and
`keios.iso`. The ISO is the bootable image that QEMU will launch. You can also
use the Python wrapper for a clean build or to choose a Make target:

```sh
python3 build.py
python3 build.py --make-target keios.elf
python3 build.py --clean-build
python3 build.py D=DEBUG
```

Pass Make options through `--make-args`, for example to choose the number of
parallel jobs:

```sh
python3 build.py --make-args -j4
```

Run `make help` to list the available targets. `make config` creates a local
`config.mk` for build configuration.

### Run the regression tests

Run the repository's unit tests from the project root:

```sh
python3 -m unittest discover -s tests -v
```

Or use the convenience target:

```sh
make test
```

The test suite currently covers the x86 ISR/exception ABI contract and checks
that the kernel boot log reports successful IDT setup and IRQ initialization.

### Run in QEMU

Build and launch the default virtual machine:

```sh
python3 run.py
```

By default, QEMU emulates an i386 machine with an N270 CPU and 4 GiB of RAM.
The runner creates an 8 GiB QCOW2 virtual disk named `disk.qcow2` when needed;
this is a file in the project directory, not a disk partition on your host.

| Option | Effect |
| --- | --- |
| `--headless` | Run QEMU without a graphical display |
| `--skip-run` | Build only; do not launch QEMU |
| `--skip-build` | Launch the existing ISO without rebuilding |
| `--clean-build` | Clean build outputs before compiling |
| `--disk-file PATH --disk-size SIZE` | Select the virtual disk and its capacity |
| `--help` | Show all supported options |

For example:

```sh
python3 run.py --headless
python3 run.py --disk-file /path/to/keios.qcow2 --disk-size 2G
```

Use `python3 run.py --help` to see all options. QEMU diagnostic output is
written to `qemu.log` and `debug.log`.

---

<div align="center">

## 🧰 Scripts

</div>

| Script | What it does | Example |
| --- | --- | --- |
| [`build.py`](./build.py) | Runs Make; can clean first and accepts Make arguments. | `python3 build.py --clean-build` |
| [`run.py`](./run.py) | Builds (unless skipped), creates a QCOW2 disk if needed, and starts QEMU. | `python3 run.py --headless` |
| [`clean.py`](./clean.py) | Removes build outputs and ignored artifacts; disk and logs are included by default. | `python3 clean.py --dry-run` |
| [`healthcheck.py`](./healthcheck.py) | Runs Rust Clippy and a build, then refreshes the README badges. | `python3 healthcheck.py` |
| [`scripts/bitmap_to_font.py`](./scripts/bitmap_to_font.py) | Converts a horizontal PNG atlas of ASCII glyphs 32–126 into Rust bitmap-font source. | `python3 scripts/bitmap_to_font.py atlas.png -o rust/crates/drivers/src/display/text/generated_font.rs` |

The font generator needs Pillow. Install its Python dependency and run the
script like this:

```sh
python3 -m pip install -r scripts/requirements.txt
python3 scripts/bitmap_to_font.py atlas.png
```

The atlas width must be divisible by 95. Font generation is not currently part
of the Make build.

> [!WARNING]
> `clean.py` removes `disk.qcow2` by default. Use `--skip-disk` to keep it, or
> `--dry-run` to preview cleanup. `make clean` removes build outputs without
> deleting the disk image or logs.

---

<div align="center">

## 🗂️ Project structure

</div>

```text
Makefile                  Primary build orchestration for the kernel and ISO
build.py                  Thin wrapper for invoking make targets
run.py                    Build + QEMU launch wrapper for local testing
clean.py                  Safe cleanup of build artifacts, logs, and disk images
healthcheck.py            README badge refresh and validation helper

src/                      C kernel sources, x86 assembly, and low-level drivers
include/                  Public headers and project configuration
bin/                      Generated object files and intermediate build outputs
rootfs/                   Files packaged into the initramfs archive
userspace/                Early userspace sources and stdlib stubs
resources/                UI/font and asset sources used by the kernel build
rust/                     Cargo workspace, nightly toolchain pin, and no_std crates
scripts/                  Python utilities such as the bitmap-to-font converter and formatting helpers
docs/                     Boot, architecture, and subsystem documentation

grub.cfg                  GRUB configuration for the ISO boot image
linker.ld                 ELF32 linker script
keios.elf                 Kernel binary produced by the build
initramfs.cpio            Generated initramfs archive
keios.iso                 Bootable ISO image
FIXME.md                  Known implementation issues and reproducibility notes
TODO.md                   Near-term project roadmap and follow-up tasks
```

---

<div align="center">

## 📚 Documentation

</div>

<div align="center">

| [Boot flow](./docs/BOOT.md) | [Architecture](./docs/ARCHITECTURE.md) | [Memory](./docs/subsystem/MEMORY.md) |
|:---:|:---:|:---:|
| [Interrupts](./docs/subsystem/INTERRUPTS.md) | [Processes](./docs/subsystem/PROCESS.md) | [System calls](./docs/subsystem/SYSCALL.md) |

</div>

Known implementation issues and follow-up work are tracked in
[`FIXME.md`](./FIXME.md) and [`TODO.md`](./TODO.md). The project is still in an
experimental stage; use the docs below as implementation notes rather than a
production-ready reference.

---

<div align="center">

**KeiOS** · Experimental systems software · [GPL-3.0](./LICENSE.md)

</div>
