# KeiOS

An experimental 32-bit operating system built to explore kernel design.
_Still in development..._

## Requirements

* Linux environment,
* C23 i686-elf cross-compiler,
* NASM, Make, GRUB, CPIO, xorriso, and mtools,
* Rust nightly with the `rust-src` component,
* Python 3 and Pillow,
* QEMU.

## How to run

Install the Python dependency:

```sh
python3 -m pip install -r scripts/requirements.txt
```

Build the kernel and bootable ISO:

```sh
make
```

Run KeiOS in QEMU:

```sh
python3 run.py
```

Run the regression tests:

```sh
make test
```

The kernel boots through GRUB, initializes CPU, memory, and ACPI, parses the
CPIO initramfs, and runs `bin/init.bin` once in Ring 3 if it is present. It
then enters a kernel-mode shell. KeiOS does not yet have process isolation,
scheduling, or general executable loading.
