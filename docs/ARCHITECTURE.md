# System Architecture

---
## Overview & Design Philosophy

This operating system is built on a **true microkernel architecture** targeting the **i386 (32-bit x86)** platform. The core philosophy is strict privilege separation: the kernel executes in Ring 0 and is responsible only for fundamental hardware primitives, while all drivers, file systems, and user interfaces execute as isolated user-space servers in Ring 3

- **Target Architecture:** i386 (x86 32-bit, Intel/AMD)
- **Kernel Model:** Microkernel
- **Bootloader:** GRUB2 (Multiboot specification compliant)
- **Executable Format:** ELF32 (Executable and Linkable Format)

---
## Privilege Ring Separation

To maximize fault tolerance and security, the system utilizes x86 hardware protection rings to isolate system components:

`````txt
+-----------------------------------------------------------------+
| USER SPACE (Ring 3)                                             |
|                                                                 |
|  +----------------+   +-----------------+   +----------------+  |
|  |   TTY Shell    |   |  Display Server |   |  Future VFS /  |  |
|  |  (Application) |   |  (Framebuffer)  |   |  EXT2 Server   |  |
|  +--------+-------+   +--------+--------+   +----------------+  |
|           |                    |                                |
|           +--- Shared Mem -----+                                |
|           |    & IPC Messages                                   |
+-----------|-----------------------------------------------------+
|           v                                                     |
| KERNEL SPACE (Ring 0)                                           |
|                                                                 |
|  +-----------------------------------------------------------+  |
|  | Microkernel Core                                          |  |
|  | - Interrupts & Exceptions (GDT, IDT, PIC/APIC)            |  |
|  | - Memory Management (PMM, VMM / i386 Paging)              |  |
|  | - Preemptive Scheduler & Task Management                  |  |
|  | - Inter-Process Communication (IPC) & Shared Memory       |  |
|  +-----------------------------------------------------------+  |
+-----------------------------------------------------------------+
`````

### Ring 0: The Microkernel Core

The kernel remains as small as possible. A bug or crash here will panic the entire system, so only minimal hardware abstraction layers exist in this ring:

- **Descriptor Tables (GDT / IDT):** Manages segmentation, user/kernel mode switching, hardware interrupts (IRQs), and CPU exception handling
- **Memory Management:**
    - **Physical Memory Manager:** Allocates and frees 4KB physical page frames using a bitmap
    - **Virtual Memory Manager:** Manages two level i386 page directories and page tables
- **Task Scheduling:** Preemptive round robin scheduler driven by the PIT timer
- **IPC Engine:** Facilitates synchronous message passing and shared memory setup between Ring 3 processes

### Ring 3: User-Space & Drivers

All non-essential services run as isolated user mode processes. If a driver crashes, the microkernel can terminate and restart it without bringing down the OS

- **Display Server:** Manages the screen, rendering text for TTY and eventually compositing graphical windows
- **Input Drivers:** PS/2 Keyboard and mouse translation servers
- **Storage & File Systems:** VFS, EXT2, and AHCI disk controllers (loaded via Multiboot `initrd` during early boot)

---
## Virtual Memory Layout

The system uses a **Higher-Half Kernel** memory model. The 4GB 32-bit virtual address space is split into a 3GB user space and a 1GB kernel space:

| **Virtual Address Range**   | **Size** | **Access Level** | **Description**                                                                                            |
| --------------------------- | -------- | ---------------- | ---------------------------------------------------------------------------------------------------------- |
| `0x00000000` - `0x00000FFF` | 4 KB     | Unmapped         | **Null Pointer Guard:** Traps null pointer dereferences                                                    |
| `0x00001000` - `0xBFFFFFFF` | ~3 GB    | Ring 3 (User)    | **User Space:** Application code, heap, stack, and shared memory regions                                   |
| `0xC0000000` - `0xC03FFFFF` | 4 MB     | Ring 0 (Kernel)  | **Kernel Code & Data:** Mapped to physical memory starting at `0x00100000` (1MB)                           |
| `0xC0400000` - `0xEF000000` | ~748 MB  | Ring 0 (Kernel)  | **Kernel Heap:** Kernel dynamic memory allocation, IPC message buffers, page tables                        |
| `0xFD000000` - `0xFFFEFFFF` | Varies   | Ring 0 / Ring 3  | **MMIO / Framebuffer:** Physical video RAM mapped via linear framebuffer, accessible by the Display Server |
| `0xFFFFF000` - `0xFFFFFFFF` | 4 KB     | Ring 0 (Kernel)  | **Recursive Page Directory:** Used for fast page table manipulation                                        |

---
## Inter-Process Communication (IPC)

Because system services are isolated in Ring 3, fast and reliable IPC is critical for performance. The microkernel provides two distinct communication mechanisms:

### Short IPC Messages (Message Passing)

Used for synchronization, signaling, and small payloads (e.g., keyboard events, VFS commands)

- Transfer small fixed-size kernel message queues via system calls (`int 0x80`)
- **Example:** The PS/2 driver sends a 4-byte scancode message to the active TTY Shell

### Shared Memory (Zero-Copy Transfer)

Used for high bandwidth data transfers where copying memory across ring boundaries would cause performance bottlenecks

- VMM maps the same physical memory frames into the virtual address spaces of two processes
- **Example:** The Display Server and an application share a memory mapped double buffer. The application draws UI elements into this shared region and sends a short IPC message: `"Flush Rect(x, y, w, h)"`. The Display Server then blits the updated pixels directly to physical video RAM

---
## Hardware Access for Ring 3 Drivers

To allow user-space drivers (like the Display Driver) to interact with physical hardware without running in Ring 0, the kernel provides controlled access exemptions:

- **Memory Mapped I/O (MMIO):** The kernel maps physical hardware memory addresses (e.g., linear framebuffer) into the virtual address space of the specific driver process
- **Port I/O Permissions:** For drivers requiring x86 `inb` / `outb` instructions (e.g., legacy VGA or PS/2 controller), the kernel alters the **I/O Permission Bitmap** in the TSS