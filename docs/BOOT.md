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
  -> interrupts enabled
  -> kernel-mode shell polls for input
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
address as needed, then initializes paging, the physical allocator from
Multiboot usable-memory ranges, and a kernel heap mapped from contiguous
allocated frames. `mem_upper` is reported as a legacy physical limit, not used
to decide which frames are available. See [the memory notes](./subsystem/MEMORY.md).

### ACPI and archive setup

The boot path searches for ACPI data, enumerates Multiboot modules, and parses
the CPIO initramfs. The kernel retains the raw bytes for `bin/init.bin`, if
present. The current boot path does not launch it.

### Hardware initialization

The kernel initializes in-kernel IPC, starts the PIT, configures the PS/2
keyboard driver, and selects the RGB framebuffer terminal when supported.
Otherwise it initializes VGA text output.

### Shell

After driver initialization, the kernel enables interrupts and starts its
kernel-mode polling shell. The call to run the bundled init program is
currently disabled; there is no functioning user-process transition,
isolation, or process manager.

## Not yet part of the boot path

The following steps are architectural goals, not completed boot stages:

- Safely load an executable from the initramfs; the existing init routine is
  not a validated ELF loader.
- Create a separate page directory and stack for each task.
- Add per-task Ring 3 entry frames and context switching.
- Start user-space display, input, filesystem, or shell services.
- Load ELF executables or provide isolated address spaces and scheduled tasks.
