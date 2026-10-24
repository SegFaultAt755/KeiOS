# Memory Management

## Current implementation

The i386 memory code provides higher-half paging helpers, a physical page-frame
bitmap, and a kernel heap. These facilities are early kernel primitives; they
do not yet provide isolated per-process address spaces.

### Virtual memory

The kernel uses 32-bit two-level x86 paging and a higher-half kernel mapping.
`vmm_map_page()`, `vmm_unmap_page()`, and `vmm_get_phys()` operate on the active
page directory through the recursive mapping. The early identity mapping is
removed during `vmm_init()`.

The intended split leaves low virtual memory for user mappings and maps the
kernel at `KERNEL_VIRTUAL_OFFSET` (`0xC0000000`). A working task manager that
creates and switches user page directories is not present.

### Physical memory

`pmm_init()` tracks frame allocation in a bitmap and reserves memory below the
kernel/module allocation boundary. The current limit is derived from
Multiboot `mem_upper`; it does not use the full Multiboot memory map to mark
only usable RAM. The interpretation and reservation of this range need review
before relying on the allocator beyond the current emulator setup.

### Kernel heap

`heap_init()`, `kmalloc()`, and `kfree()` manage variable-size blocks in a
kernel heap initialized during `mem_init()`. Rust allocations use this heap via
the drivers crate's `GlobalAlloc` implementation. The heap alignment and
allocation arithmetic have known correctness concerns; see `FIXME.md` before
extending allocator use.

## Status

| Component | Current status |
| --- | --- |
| Higher-half kernel paging and recursive mapping | Implemented; not comprehensively validated |
| Physical frame bitmap | Implemented; usable-range discovery is incomplete |
| Kernel heap | Implemented; alignment/overflow concerns remain |
| Per-task address spaces and user/kernel isolation | Not implemented |
| Framebuffer/MMIO mapping for an isolated user driver | Not implemented |

## Follow-up

Use the Multiboot memory map to identify usable ranges, reserve every live
boot-time structure and module, validate arithmetic and address ranges, and
make the heap satisfy the alignment contract required by Rust `GlobalAlloc`.
Only then should process-specific page directories and user mappings be exposed.
