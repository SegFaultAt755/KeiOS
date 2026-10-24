# KeiOS Architecture: Current State

## Scope

KeiOS targets 32-bit x86 (i386), boots through GRUB using the Multiboot
interface, and combines a freestanding C23 kernel with a Rust `no_std` drivers
crate. The Rust library is statically linked into the kernel; it is not a
collection of isolated user-space services.

The current kernel is a single privileged kernel with early subsystem
boundaries, not yet a functioning microkernel. The Ring 3 process, service, and
driver model described in earlier designs is a roadmap.

## Implemented boot-time shape

```text
GRUB / Multiboot
      |
      v
x86 entry and higher-half mappings
      |
      v
GDT, IDT, CPU checks, memory initialization
      |
      v
ACPI and Multiboot module / CPIO initialization
      |
      v
PIT, PS/2 keyboard, framebuffer-or-VGA terminal
      |
      v
Kernel-mode polling shell
```

The boot path is detailed in [BOOT.md](./BOOT.md). The source entry point is
`kernel_entry()` in `src/kernel/boot/entry.c`.

## Current components

- **CPU and interrupts:** GDT/TSS setup, IDT gates, legacy PIC remapping, ISR and
  IRQ stubs, a PIT timer, and an `int 0x80` syscall entry.
- **Memory:** Higher-half paging, page mapping helpers, a bitmap physical frame
  allocator, and a variable-size kernel heap. The allocator currently uses
  `mem_upper` rather than the full Multiboot memory map; see
  [MEMORY.md](./subsystem/MEMORY.md).
- **Drivers and output:** PS/2 keyboard input; VGA text output; an RGB linear
  framebuffer path backed by Rust display routines; and fallback to VGA.
- **Storage input:** GRUB loads the CPIO initramfs as a Multiboot module. The
  kernel invokes the Rust CPIO parser and visits archive entries.
- **IPC and syscalls:** A fixed-size in-kernel endpoint/message queue and
  syscall indices 0–5. They do not provide process isolation or a complete
  userspace service model.
- **Shell:** A built-in command loop polls keyboard input and runs commands in
  kernel mode. It is not an executable loaded from the initramfs.

## Not implemented yet

- Independent user address spaces and reliable Ring 3 task startup.
- A process control block, scheduler, context switching, and service restart.
- General ELF loading or execution of the bundled init program.
- User-space drivers, a VFS, and a complete IPC-based display/input stack.
- Windows PE or Linux executable compatibility, including the planned dual
  syscall ABI.

The existence of a stub, interface, or design document does not imply the
feature is complete. `FIXME.md` records known implementation and reproducibility
issues.

## Directory guide

- `src/arch/x86/`: x86 startup, CPU/interrupt code, and paging/heap primitives.
- `src/kernel/`: boot orchestration, ACPI, archive integration, shell, and
  kernel services.
- `src/drivers/`: C hardware and terminal drivers.
- `rust/crates/drivers/`: Rust display, CPIO, and IPC code linked through C ABI
  wrappers.
- `userspace/` and `rootfs/`: early user program sources and initramfs contents;
  the current boot flow does not execute those programs.

## Planned direction

The project aims to evolve toward isolated tasks and a small kernel service
interface, with a longer-term experiment in supporting multiple executable
ecosystems. Those goals require a reliable interrupt/exception ABI, safe memory
management, process loading, and a tested userspace transition before the
design can be described as a microkernel or dual-ABI operating system.
