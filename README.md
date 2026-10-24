<div align="center">

<img src="./keios-banner.svg" alt="KeiOS" width="720">

<br>

![i386](https://img.shields.io/badge/Architecture-i386-2563eb?logo=intel&logoColor=white)
![C23](https://img.shields.io/badge/Kernel-C23-0f766e)
![Rust nightly](https://img.shields.io/badge/Rust-nightly-d97706?logo=rust&logoColor=white)
![GPLv3](https://img.shields.io/badge/License-GPLv3-15803d)

<!-- healthcheck:badges:start -->
![Clippy](https://img.shields.io/badge/Clippy-Undefined-black) ![Build](https://img.shields.io/badge/Build-Undefined-black)
<!-- healthcheck:badges:end -->

### An experimental 32-bit operating system, built to explore kernel design.

[Getting Started](#-getting-started) · [Current Capabilities](#-current-capabilities) · [Helper Scripts](#-helper-scripts) · [Documentation](#-documentation)

</div>

---

## Project at a glance

KeiOS is an experimental **i386 operating system** combining a freestanding
C23 kernel with a Rust `no_std` drivers library. Today, it boots through GRUB,
initializes core CPU and memory facilities, reads a CPIO initramfs, brings up
basic hardware, and presents an interactive kernel shell.

> [!NOTE]
> KeiOS is an early-stage research project, not a general-purpose operating
> system. Process isolation, general ELF execution, and Windows or Linux binary
> compatibility are future goals—not current features.

<div align="center">

| **Target** | **Kernel** | **Boot** | **License** |
|:---:|:---:|:---:|:---:|
| 32-bit x86 | C23 + Rust | GRUB / Multiboot | GPL-3.0 |

</div>

## ✨ Current capabilities

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

## 🚀 Getting started

### Requirements

The supported build and run workflow is Linux or Linux under WSL2. Install:

- GNU Make, NASM, `i686-elf-gcc` and `i686-elf-ld` (or a compatible
  32-bit freestanding toolchain).
- Rust nightly through `rustup`, with the `rust-src` component.
- `grub-mkrescue`, `xorriso`, `mtools`, and `cpio`.
- Python 3 and QEMU (`qemu-system-i386`, `qemu-img`).

On Debian or Ubuntu, install the common host packages:

```bash
sudo apt update
sudo apt install build-essential nasm binutils cpio xorriso mtools \
  grub-common grub-pc-bin qemu-system-x86 qemu-utils python3
```

Install an i686-elf cross-compiler using your distribution’s packages or build
the GNU toolchain. Set up the Rust toolchain with:

```bash
rustup toolchain install nightly
rustup component add rust-src --toolchain nightly
```

### Build

Build the kernel, initramfs, and bootable ISO:

```bash
make
```

The default target produces `keios.elf`, `initramfs.cpio`, and `keios.iso`.
Alternatively, use the Python wrapper to pass a Make target or build options:

```bash
python3 build.py
python3 build.py --make-target keios.elf
python3 build.py --clean-build
python3 build.py D=DEBUG
```

For Make command-line options, pass them through `--make-args`:

```bash
python3 build.py --make-args -j4
```

Run `make help` to see Make targets. `make config` creates a local `config.mk`
for build configuration.

### Run in QEMU

Build and launch the default virtual machine (i386, N270 CPU, 4 GiB RAM, and an
8 GiB QCOW2 disk created when needed):

```bash
python3 run.py
```

| Option | Effect |
| --- | --- |
| `--headless` | Run QEMU without a graphical display |
| `--skip-run` | Build only; do not launch QEMU |
| `--skip-build` | Launch the existing ISO without rebuilding |
| `--clean-build` | Clean build outputs before compiling |
| `--disk-file PATH --disk-size SIZE` | Select the virtual disk and its capacity |
| `--help` | Show all supported options |

For example:

```bash
python3 run.py --headless
python3 run.py --disk-file /path/to/keios.qcow2 --disk-size 2G
```

QEMU diagnostic output is written to `qemu.log` and `debug.log`.

## 🧰 Helper scripts

| Script | What it does | Example |
| --- | --- | --- |
| [`build.py`](./build.py) | Runs Make; can clean first and accepts Make arguments. | `python3 build.py --clean-build` |
| [`run.py`](./run.py) | Builds (unless skipped), creates a QCOW2 disk if needed, and starts QEMU. | `python3 run.py --headless` |
| [`clean.py`](./clean.py) | Removes build outputs and ignored artifacts; disk and logs are included by default. | `python3 clean.py --dry-run` |
| [`healthcheck.py`](./healthcheck.py) | Runs Rust Clippy and a build, then refreshes the README badges. | `python3 healthcheck.py` |
| [`helpers/bitmap_to_font.py`](./helpers/bitmap_to_font.py) | Converts a horizontal PNG atlas of ASCII glyphs 32–126 into Rust bitmap-font source. | `python3 helpers/bitmap_to_font.py atlas.png -o rust/crates/drivers/src/display/text/generated_font.rs` |

The font generator needs Pillow:

```bash
python3 -m pip install -r helpers/requirements.txt
python3 helpers/bitmap_to_font.py atlas.png
```

The atlas width must be divisible by 95. Font generation is not currently part
of the Make build.

> [!WARNING]
> `clean.py` removes `disk.qcow2` by default. Use `--skip-disk` to keep it, or
> `--dry-run` to preview cleanup. `make clean` removes build outputs without
> deleting the disk image or logs.

## 🗂️ Project structure

```text
include/                  C interfaces and configuration
src/                      C and x86 assembly kernel and drivers
userspace/                Early userspace sources (not launched at boot)
rootfs/                   Files packaged into the CPIO initramfs
rust/crates/drivers/       Rust no_std driver and support library
helpers/                   Asset-generation helper
docs/                      Boot, architecture, and subsystem documentation
build.py / run.py          Build and QEMU wrappers
clean.py                   Build-artifact cleanup
```

## 📚 Documentation

<div align="center">

| [Boot flow](./docs/BOOT.md) | [Architecture](./docs/ARCHITECTURE.md) | [Memory](./docs/subsystem/MEMORY.md) |
|:---:|:---:|:---:|
| [Interrupts](./docs/subsystem/INTERRUPTS.md) | [Processes](./docs/subsystem/PROCESS.md) | [System calls](./docs/subsystem/SYSCALL.md) |

</div>

Known implementation issues and follow-up work are tracked in
[`FIXME.md`](./FIXME.md).

---

<div align="center">

**KeiOS** · Experimental systems software · [GPL-3.0](./LICENSE.md)

</div>
