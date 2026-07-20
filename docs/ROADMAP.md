# OS Development Roadmap

---
## 📋 Task Status Summary

| Subsystem                  | Execution Ring   | Status         | Notes                                           |
| :------------------------- | :--------------- | :------------- | :---------------------------------------------- |
| **Boot & Core Memory**     | Ring 0           | 🟢 Completed   | Multiboot2, PMM, VMM & `kmalloc` done           |
| **Interrupts / GDT**       | Ring 0           | 🟢 Completed   | IDT, PIC remapping, Exception handling done     |
| **Kernel Display & Shell** | Ring 0 (Temp)    | 🟡 In Progress | Static driver & kernel TTY shell working        |
| **Ramdisk & ELF Loader**   | Ring 0           | 🟡 In Progress | Active focus: parsing `initrd` & ELF32 binaries |
| **Ring 3 Jump & IPC**      | Ring 0 -> Ring 3 | 🟡 In Progress | TSS setup, `IRET` stack frame, system calls     |
| **User Space Drivers**     | Ring 3           | 🔴 Planned     | Display Server & PS/2 driver refactor           |
| **VFS / EXT2 / Disk**      | Ring 3           | 🔴 Planned     | Storage & Filesystem                            |
| **Multitasking Scheduler** | Ring 0           | 🔴 Planned     | Add multitasking & GUI compositor support       |

---
## Core Kernel & Memory `[COMPLETED]`
_Target: Establish stable Ring 0 memory, interrupt handling, and basic in kernel UI_

- [x] Multiboot2 handshake & Protected Mode entry
- [x] Temporary higher-half paging setup (`0xC0000000+`)
- [x] **PMM:** Bitmap allocator for 4KB physical page frames
- [x] **VMM:** Page directory manipulation, recursive mapping at `0xFFFFF000`, `kmalloc`/`kfree` dynamic allocator
- [x] **IDT & Exceptions:** Catch Page Faults (`#PF`), General Protection Faults (`#GP`), and remap PIC interrupts
- [x] **In Kernel Display Driver:** Static Ring 0 display driver
- [ ] **TTY:** Early shell for input/output verification

---
## Initrd, Monotasking & Ring 3 Transition `[IN PROGRESS]`
_Target: Parse Ramdisk binaries, load ELF32 programs, and drop CPU privileges to Ring 3_

- [ ] **Initrd Parsing:** Parse Multiboot2 module tags (TAR/raw module) to locate program binaries in RAM
- [ ] **ELF32 Loader:** Parse headers and map code/data segments into user virtual space
- [ ] **GDT & TSS:** Configure Ring 3 code/data selectors and TSS for kernel stack switching
- [ ] **User Mode Switch:** Build initial kernel stack frame (`SS`, `ESP`, `EFLAGS`, `CS`, `EIP`) and jump to Ring 3 via `IRET`
- [ ] **System Calls & IPC Engine:** Implement `int 0x80` entry point and short message passing / shared memory primitives

---
## Ring 3 Driver Servers & User TTY `[PLANNED]`
_Target: Move drivers out of Ring 0 into isolated user-space services communicating via IPC_

- [ ] **Display Server (Ring 3):**
    - MMIO framebuffer mapping
    - IPC handler for screen flushing & double buffering
- [ ] **PS/2 Keyboard Driver (Ring 3):**
    - Configure TSS I/O Permission Bitmap for ports `0x60`/`0x64`
    - Scancode translation & IPC messaging to active TTY
- [ ] **TTY Shell (User Application):**
    - Refactor shell into a standalone Ring 3 user application communicating strictly via IPC

---
## Storage & Filesystems `[PLANNED]`
_Target: Replace `initrd` with persistent storage and filesystem access_

- [ ] **VFS Server:** Unified file operations (`open`, `read`, `write`, `close`)
- [ ] **Disk Controller Driver:** AHCI driver running in user space (MMIO/Port access)
- [ ] **EXT2 Server:** Read/write parsing for EXT2 partitions

---
## Multitasking & Extended Subsystems `[PLANNED]`
_Target: Evolve from monotasking to a full-fledged multi-process environment_

- [ ] **Preemptive Scheduler:** PIT driven round-robin task switching & context switching
- [ ] **GUI Compositor:** Expand Display Server to manage overlapping windows & cursor
- [ ] **Network Server:** Network card driver (e.g., RTL8139 / E1000) & TCP/IP stack
- [ ] **PC Speaker Driver:** Sound playback server
