# FIXME

Concrete correctness and reproducibility issues found during the codebase review.

## Boot and input path
- **User mode can invoke every exception and IRQ IDT gate.** `idt_init()` installs all exception and remapped IRQ gates with attributes `0xEE` (present, DPL 3), not just the syscall gate at vector 128. Once Ring 3 code runs, software `int` instructions can enter these handlers; many paths panic the whole kernel. Use kernel-only privilege levels for exception/IRQ gates and grant DPL 3 only to intentional syscall gates.

## Memory mapping and generated inputs

- **ACPI root-table mappings cover only the first page.** `acpi_init()` maps a single page containing the root table header, while `find_fadt()` walks entries across the table's declared length. The FADT pointer is then dereferenced without an explicit mapping/reservation step. Map and validate the complete table ranges before parsing them. `parse_dsdt()` already maps the DSDT pages, but should follow the same checked-range approach.
- **ACPI shutdown's `_S5_` scan can read beyond the AML buffer.** `acpi_shutdown()` subtracts the ACPI header size without first validating `header.length`, then scans for a four-byte signature and reads a package and values without checking each access against the remaining AML length. Validate the DSDT length and parse the AML package with bounds checks before using the sleep values.
- **ACPI mode enable can spin forever.** `enable_acpi_mode()` waits for `SCI_EN` in an unbounded loop after issuing the SMI command. Add a bounded timeout and report a useful failure rather than hanging boot indefinitely.
- **The Rust display font is required by a clean build but ignored by Git.** `rust/crates/drivers/src/display/text/fonts.rs` includes `generated_font.rs`; `.gitignore` excludes that file, and the Makefile does not generate it. A fresh checkout therefore depends on a local generated artifact. Either track the generated source or wire generation from the tracked font asset into the build.
- **Malformed CPIO archives can be reported as successful parses.** `CpioArchive::next_entry()` returns `None` for both a normal end/trailer and invalid or truncated data, and `cpio_parse()` returns success after its loop in either case. Distinguish end-of-archive from parse errors and propagate malformed-input failures to the C caller.
- **The physical allocator treats all RAM below `mem_upper` as usable.** `mem_init()` uses only Multiboot `mem_upper`; `pmm_init()` marks every frame from `physical_alloc_start` to that limit free. This ignores the Multiboot memory map and can allocate reserved holes or device regions on non-QEMU firmware. Use the memory map as the source of usable ranges and reserve every boot structure/table the kernel continues to reference. Also account for `mem_upper` being measured above the first MiB when calculating the physical limit.
- **The kernel heap does not provide its advertised 16-byte alignment.** `struct heap_segment` is 12 bytes on i386, and `kmalloc()` returns `segment + sizeof(struct heap_segment)`, so even an aligned segment header yields a payload misaligned by 4 bytes. Fix the header/payload layout and check alignment arithmetic for overflow before exposing the allocator through Rust `GlobalAlloc`.

## Obsolete framebuffer compatibility API

## Note on the previous memory-mapping note

The old note quoted the ACPI `vmm_map_page()` arguments in the wrong order. The current call in `src/kernel/acpi/acpi.c` passes virtual address first and physical address second; the outstanding issue is the incomplete range mapping described above.
