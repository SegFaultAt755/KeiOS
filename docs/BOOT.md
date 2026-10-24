# Boot Flow

This page describes the path implemented by the current source, followed by
work that remains before the kernel can start an isolated user process.

## Current boot flow

```text
BIOS / QEMU
  -> GRUB loads Multiboot kernel and initramfs module
  -> x86 assembly entry establishes early mappings and calls kernel_entry()
  -> kernel validates Multiboot inputs
  -> GDT, IDT, and CPU feature initialization
  -> higher-half Multiboot pointer and memory initialization
  -> ACPI initialization
  -> module scan and CPIO archive parsing
  -> IPC, PIT, PS/2, and display/terminal initialization
  -> interrupts enabled; kernel shell polls for input
```

The GRUB configuration is in the repository root at `grub.cfg`. It loads
`/boot/keios.elf` and supplies `/boot/initramfs.cpio` as a module named
`initramfs`.

## Stages

### Firmware and GRUB

The build creates a GRUB ISO. GRUB loads the Multiboot kernel and CPIO module
and passes the Multiboot magic value and information pointer to the kernel
entry. The kernel checks the magic value before continuing.

### Early CPU setup

`boot_init_early_cpu()` initializes the GDT, IDT, and CPU feature checks. The
early assembly entry is responsible for setting up the initial higher-half
execution environment before calling the C entry point.

### Memory setup

The kernel converts the Multiboot information pointer to its higher-half
address as needed, then initializes paging, the physical allocator, and the
kernel heap. Current physical-memory discovery uses the Multiboot
`mem_upper` field rather than the complete memory map; memory initialization is
therefore experimental. See [the memory notes](./subsystem/MEMORY.md).

### ACPI and archive setup

The boot path searches for ACPI data, enumerates Multiboot modules, and parses
the CPIO initramfs. The archive callback identifies `bin/init.bin`.

### Hardware initialization

The kernel initializes in-kernel IPC, starts the PIT, configures the PS/2
keyboard driver, and selects the RGB framebuffer terminal when supported.
Otherwise it initializes VGA text output.

## Not yet part of the boot path

The following steps are architectural goals, not completed boot stages:

- Validate and load ELF program segments from the initramfs.
- Create a separate page directory and stack for each task.
- Set up a reliable Ring 3 entry frame and context-switching path.
- Start user-space display, input, filesystem, or shell services.
- Launch the bundled init program automatically.
