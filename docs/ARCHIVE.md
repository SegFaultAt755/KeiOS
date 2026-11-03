# Archive and Continuation Guide

## Project status

KeiOS is an archived experiment in operating-system and driver development.
It is a learning project, not a production OS, and has no practical deployment
purpose or supported real-world use case. Do not use it for real workloads.
The repository is kept as a record of the experiment and as a starting point
for anyone who wants to study or continue it; ongoing maintenance and feature
work are not expected.

KeiOS currently boots a 32-bit x86 kernel through GRUB/Multiboot. The kernel
is a single privileged C23 kernel with Rust `no_std` code linked into it. It
has early CPU, memory, ACPI, CPIO, display, keyboard, IPC, and shell components,
but it is not a microkernel or a usable general-purpose operating system.
In particular, the normal boot path ends in a kernel-mode shell. An init
binary may be packaged in the initramfs, but launching it in Ring 3 is
disabled. There is no process manager, scheduler, general executable loader,
or isolated user space.

See [ARCHITECTURE.md](./ARCHITECTURE.md), [BOOT.md](./BOOT.md), and
[PROCESS.md](./subsystem/PROCESS.md) for implemented behavior and limitations.
Treat roadmap items and stubs as plans, not working features.

## Where to resume

If development is intentionally resumed, start from the current `main` branch
and re-establish a clean baseline before adding features:

1. Read [CONTRIBUTING.md](../CONTRIBUTING.md) and the architecture, boot, and
   process notes linked above.
2. Run `make test`, the Rust formatting and Clippy checks, and
   `python3 build.py --clean-build` where the required cross-toolchain and host
   tools are available. Record existing failures rather than assuming a clean
   build.
3. Use the "Recheck before enabling concurrency or adding backends" section at
   the top of [TODO.md](../TODO.md) as the current technical entry point. Its
   first items describe trust-boundary and safety problems, not optional
   polish.
4. Before enabling the init path, replace the raw flat-binary loader with
   validated executable loading, explicit page permissions and address-space
   layout, a protected null page, BSS handling, and a well-defined user stack.
   Validate syscall buffers and IPC endpoint ownership before exposing those
   interfaces to Ring 3.
5. Before adding concurrency, define synchronization for IPC and global Rust
   state. Add tests for malformed inputs and failure paths before extending
   hardware backends or building a scheduler.
6. Only after these foundations are tested should work proceed to process
   control blocks, isolated address spaces, context switching, and scheduling.
   Update the architecture and boot docs whenever implemented behavior
   changes.

The usual local checks are listed in [README.md](../README.md). A successful
build or emulator boot alone does not demonstrate process isolation, user-mode
safety, or production readiness.
