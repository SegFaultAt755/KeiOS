# KeiOS Agent Instructions

## Project architecture

KeiOS is an experimental 32-bit x86 (i386) operating system. It boots through
GRUB/Multiboot into a freestanding C23 kernel, with a Rust `no_std` drivers
library statically linked into the kernel. The current boot flow initializes
CPU and memory facilities, ACPI, the CPIO initramfs, basic drivers, and a
kernel-mode shell. It is not yet a microkernel: process isolation, a scheduler,
general executable loading, and user-space services are not implemented.

- `src/arch/x86/`: architecture entry, CPU/interrupt code, paging, and heap.
- `src/kernel/`: boot orchestration and kernel services, shell, and archive
  integration.
- `src/drivers/`: C hardware and terminal drivers.
- `rust/crates/drivers/`: Rust display, CPIO, and IPC code, including C ABI
  boundaries.
- `include/`: C headers and shared configuration.
- `userspace/` and `rootfs/`: early program sources and initramfs inputs; the
  normal boot path does not execute these programs.
- `docs/`: architecture, boot, and subsystem documentation.
- `tests/`: Python regression tests.

Read [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) and, for boot-path work,
[docs/BOOT.md](docs/BOOT.md), before changing system boundaries or describing
implemented behavior. Treat design documents and stubs as plans, not proof that
a feature works.

## Workflow

1. Read the relevant source, tests, and documentation before editing. Follow
   [CONTRIBUTING.md](CONTRIBUTING.md), including its C/Rust interoperability,
   dependency, and contribution requirements.
2. Keep changes focused and preserve the C23 + Rust approach; do not introduce
   C++.
3. Update the relevant documentation whenever behavior, architecture,
   interfaces, build steps, or tests change. Keep implementation claims
   consistent with the current code.
4. Run the focused tests and checks below, plus any checks required by the
   change. Report failures rather than implying a check passed.
5. Commit each completed task by default, unless the user asks not to commit.
   Use a clear, task-specific commit message. Stage only the files belonging to
   the task; never include unrelated or pre-existing work, and do not amend
   existing commits.

## Tests and checks

Run from the repository root:

```sh
# Python regression tests
make test
# Equivalent:
python3 -m unittest discover -s tests -v

# Rust formatting and lint checks
cd rust && cargo fmt --all -- --check
cd rust && cargo clippy -- -A clippy::all -D clippy::correctness

# Full build to a bootable ISO
python3 build.py --clean-build
```

The default Make target builds `keios.elf`, `initramfs.cpio`, and
`keios.iso`. A clean build requires the host tools and cross-compiler described
in [README.md](README.md). When available, validate the kernel format and
Multiboot header as CI does:

```sh
file keios.elf
file keios.iso
grub-file --is-x86-multiboot keios.elf
```

`python3 healthcheck.py` runs the Rust Clippy and build health checks and
refreshes the build-status badges in `README.md`; inspect its resulting changes
before committing. Format C with `clang-format` using the root
`.clang-format` file. For Python scripts, follow the formatting guidance in
[CONTRIBUTING.md](CONTRIBUTING.md).

Regression tests currently verify the x86 ISR/exception ABI and expected
interrupt initialization messages in `debug.log`. Add or update focused tests
for behavior changes, respecting the project’s test contribution policy.

## Documentation

- [README.md](README.md): setup, build, test, run, and project structure.
- [CONTRIBUTING.md](CONTRIBUTING.md): engineering principles and contribution
  workflow.
- [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md): implemented architecture and
  limitations.
- [docs/BOOT.md](docs/BOOT.md): current boot sequence and incomplete stages.
- [docs/subsystem/](docs/subsystem/): subsystem notes, including memory,
  interrupts, processes, and syscalls.
