# Memory Management

## Current implementation

The i386 memory code provides higher-half paging helpers, a physical page-frame
bitmap, and a kernel heap. These facilities are early kernel primitives; they
do not yet provide isolated per-process address spaces.

### Virtual memory

The kernel uses 32-bit two-level x86 paging and a higher-half kernel mapping.
`vmm_map_page()` operates on the active page directory through the recursive
mapping, preserving the bootstrap large-page mappings where they already cover
the requested physical page. The early identity mapping is removed during
`vmm_init()`. ACPI tables are mapped on demand in a separate, bounded kernel
virtual window at `0xD0000000`–`0xE0000000`; the heap begins at
`0xE0000000`, and the framebuffer uses a separate window beginning at
`0xF0000000`.

The intended split leaves low virtual memory for user mappings and maps the
kernel at `KERNEL_VIRTUAL_OFFSET` (`0xC0000000`). A working task manager that
creates and switches user page directories is not present.

### Physical memory

`pmm_init()` uses the variable-length Multiboot memory map as the source of
usable frames. It frees only complete pages in type-1 available ranges, then
reserves overlapping non-available ranges, the kernel, Multiboot structures,
module descriptors and payloads, and the framebuffer. The Multiboot `mem_upper`
limit is measured above the first MiB and is logged only for comparison; it is
not used as a substitute for the memory map. The map and boot structures must
remain inside the 16 MiB bootstrap mapping until paging is initialized.

### Kernel heap

`heap_init()`, `kmalloc()`, and `kfree()` manage variable-size blocks in a
kernel heap initialized during `mem_init()`. The physical allocator reserves a
contiguous range for the heap and maps it at `KERNEL_HEAP_VIRTUAL_START`.
Payload addresses are 16-byte aligned; the Rust `GlobalAlloc` implementation
rejects layouts requiring stronger alignment.

The linker includes compiler-generated section suffixes, including `.bss.*`,
before defining `_kernel_end`. This keeps statically allocated Rust state
inside the physical range reserved for the kernel and prevents the frame
allocator from reusing those pages for page tables or other allocations.

## Status

| Component | Current status |
| --- | --- |
| Higher-half kernel paging and recursive mapping | Implemented; not comprehensively validated |
| Physical frame bitmap | Implemented; driven by the Multiboot memory map |
| Kernel heap | Implemented; 16-byte alignment and checked allocation arithmetic |
| Per-task address spaces and user/kernel isolation | Not implemented |
| Framebuffer/MMIO mapping for an isolated user driver | Not implemented |

## Follow-up

Memory-map parsing and boot-time reservations are in place, but the allocator
and paging paths remain experimental and do not yet support process-specific
page directories or user mappings.
