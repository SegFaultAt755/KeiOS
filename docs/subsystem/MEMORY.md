# Subsystem Deep-Dive into Memory Management

---
## Overview & Architecture

The memory management subsystem operates on a **Higher-Half Kernel** model for the 32-bit x86 (i386) architecture. It is divided into three distinct abstraction layers:

1. **Physical Memory Manager:** Manages 4 KB physical memory frames using a low overhead bitmap allocator
2. **Virtual Memory Manager:** Handles two-level x86 page tables, page mapping, unmapping, and recursive page directory management
3. **Kernel Dynamic Allocator:** Manages variable-sized kernel heap allocations (`kmalloc` / `kfree`)

`````txt
+-------------------------------------------------------------------+
| Kernel Dynamic Allocator (kmalloc / kfree)                        |
+-------------------------------------------------------------------+
    |
    v
+-------------------------------------------------------------------+
| Virtual Memory Manager (VMM - 2 Level i386 Page Directories)      |
+-------------------------------------------------------------------+
    |
    v
+-------------------------------------------------------------------+
| Physical Memory Manager (PMM - Frame Allocation Bitmap)           |
+-------------------------------------------------------------------+
    |
    v
+-------------------------------------------------------------------+
| Hardware Memory (4 KB Physical Page Frames)                       |
+-------------------------------------------------------------------+
`````

---
## Subsystem Status

| **Module**                | **Level**             | **Status**     | **Description**                                   |
| ------------------------- | --------------------- | -------------- | ------------------------------------------------- |
| **Physical Memory (PMM)** | Kernel Primitive      | 🟢 Completed   | Frame allocation, free list bitmap parsing        |
| **Virtual Paging (VMM)**  | Hardware Abstractions | 🟢 Completed   | Higher half mapping, recursive page directory     |
| **Kernel Heap Allocator** | Dynamic Memory        | 🟢 Completed   | Heap expansion, `kmalloc` & `kfree` support       |
| **User Space Isolation**  | Security              | 🟡 In Progress | Per process page directory creation & Ring 3 flag |

---
## Physical Memory Manager (PMM) `[COMPLETED]`

The PMM treats RAM as an array of 4 KB physical page frames. Frame allocation status is tracked using a single bit per frame inside a dynamic bit array (bitmap)

- **Page Frame Size:** 4096 Bytes (4KB)
- **Bitmap Scale:** 1 byte of bitmap tracks (8 x 4KB = 32KB) of physical RAM
- **Initialization:** Parsed directly from the Multiboot `mmap` tag provided by GRUB2

### Bitmap Bit Legend

- `0`: Page Frame is **Free** and available for allocation
- `1`: Page Frame is **Used** (Reserved by BIOS/kernel or allocated)

---
## Virtual Memory Manager (VMM) `[COMPLETED]`

The VMM configures standard x86 32-bit paging using a two tier hierarchy: **Page Directories (PD)** and **Page Tables (PT)**

### Virtual Address Translation

`````txt
 31                  22 21                  12 11                   0
+----------------------+----------------------+-----------------------+
| Directory Index (10) |  Table Index (10)    |  Offset in Frame (12) |
+----------------------+----------------------+-----------------------+
          |                      |                        |
          v                      v                        v
  +----------------+      +-------------+        +----------------+
  | Page Directory | ---> | Page Table  | -----> | Physical Frame |
  +----------------+      +-------------+        +----------------+
`````

### Recursive Page Directory Mapping

To manipulate page tables without needing to map each page table into continuous virtual memory, the last entry (Index 1023) of the Page Directory points directly to physical address of the Page Directory itself

- **Recursive Virtual Base:** `0xFFFFF000`
- **Effect:** The Page Directory is accessible at `0xFFFFF000`, and all Page Tables can be modified directly via high virtual addresses in `0xFFC00000` - `0xFFFFFFFF`

---
## Kernel Heap (`kmalloc` / `kfree`) `[COMPLETED]`

The kernel heap resides in higher half virtual memory starting at `0xC014B000`

- **Allocation Strategy:** Block-header implicit list allocator
- **Header Structure:**
`````c
struct heap_segment {
    uint32_t len_flags; /* Length upper 28 bits, flags lower 4 bits */
    struct heap_segment *next;
    struct heap_segment *prev;
};
`````
- **Heap Expansion:** When `kmalloc` exhausts contiguous heap memory, the VMM requests extra frames from the PMM and maps new pages sequentially to grow the heap end pointer
