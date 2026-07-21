# Boot Process Specification

---

## Boot Flow Overview

This document describes the step by step execution path of the operating system from initial hardware power on to user-space execution in Ring 3

To keep this documentation easily maintainable as development progresses, steps are marked with status flags:

- `[DONE]` Fully implemented and tested
- `[IN PROGRESS]` Currently being refactored or implemented
- `[PLANNED]` Target architecture for upcoming milestones

`````txt
[Firmware & Bootloader] ---> Multiboot bootloader [DONE]
[Early Kernel Entry] ------> Setup temp paging & jump to Higher-Half [DONE]
[Microkernel Core Setup] --> GDT, IDT, PMM, VMM initialization [IN PROGRESS]
[Module Loading] ----------> Parse initrd & prepare ELF32 tasks [PLANNED]
[Ring 3 Transition] -------> Trap frame setup & IRET execution [PLANNED]
[Early User Space] --------> Display Server & TTY Shell startup [PLANNED]
`````

---

## Detailed Boot Stages

### Firmware & Bootloader `[DONE]`

1. **Power-On Self-Test (POST):** BIOS locates the boot device and executes the GRUB2 bootloader
2. **GRUB2 Handshake:** GRUB reads `grub.cfg`, switches the CPU to 32-bit Protected Mode, and parses the Multiboot header
3. **Module Loading:** GRUB loads the kernel executable into physical memory at `0x00100000` (1MB) along with any secondary modules (e.g., initial ramdisk / `initrd`)
4. **Register State on Entry:**
   - `EAX`: Multiboot Magic Number
   - `EBX`: Physical address of the Multiboot Information Structure

### Early Kernel Setup `[DONE]`

1. **Verify Multiboot Header:** Confirm `EAX` contains the correct Multiboot magic number
2. **Setup Temporary Page Directory:**
   - Create early page tables mapping virtual `0x00000000 - 0x003FFFFF` and `0xC0000000 - 0xC03FFFFF` to physical `0x00000000`
   - Enable Paging by setting the `PG` bit in CR0
3. **Higher-Half Jump:** Jump from the identity-mapped low virtual address to the high virtual address (`0xC0000000+`)
4. **Stack Pointer Setup:** Point `ESP` to the kernel stack in higher-half memory
5. **Call C Kernel Entry Point:** Pass the pointer to the Multiboot structure to `kernel_entry()`

### Microkernel Core Initialization `[IN PROGRESS]`

1. **GDT & Interrupt Setup:**
   - Initialize Global Descriptor Table (GDT) with Kernel/User Code and Data segments.
   - Load the Task State Segment (TSS) into the GDT (required for handling Ring 3 -> Ring 0 stack switches)
   - Initialize Interrupt Descriptor Table (IDT) and remap the PIC to handle hardware IRQs (keyboard, timer)
2. **Memory Subsystems:**
   - **Physical Memory Manager:** Parse the Multiboot memory map tag to initialize the physical page bitmap
   - **Virtual Memory Manager:** Unmap early identity page tables and initialize the higher-half kernel heap allocator (`kmalloc` & `kfree`)
3. **Multiboot Tag Parsing:**
   - Locate the linear **Framebuffer Info Tag** (resolution, pitch, physical address)
   - Locate the **`initrd` Module Tag** (physical start and end addresses)

### User-Space Module Loading `[PLANNED]`

1. **Parse Ramdisk (`initrd`):** Extract the ELF32 binaries for the early user-space servers (Display Server & Shell)
2. **Create Task Memory Space:**
   - Allocate a new Page Directory for the Display Driver task
   - Copy kernel higher-half mappings (`0xC0000000+`) into the task's page directory so kernel system calls remain accessible
   - Map the ELF segments into the user space (`0x00001000 - 0xBFFFFFFF`)
3. **Hardware Permission Mapping (Display Driver):**
   - **MMIO Mapping:** Map the physical framebuffer address (from Multiboot tag) into the driver's page directory
   - **Port Access:** Configure the TSS **I/O Permission Bitmap** to allow the driver access to VBE/display ports

### Transition to User Mode (Ring 3) `[PLANNED]`

1. **Prepare Trap Frame:** Push the initial Ring 3 register frame onto the kernel stack
2. **Execute `IRET` Instruction:** Pop the trap frame to drop CPU privileges from Ring 0 to Ring 3

### Early User Space Execution `[PLANNED]`

1. **Display Server Init:**
   - Takes control of the mapped framebuffer
   - Sets up shared memory regions for rendering
   - Enters an IPC event loop listening for draw requests
2. **TTY Shell Startup:**
   - Bootstraps in user mode
   - Sends text print requests via IPC to the Display Server
   - Awaits user keypresses via keyboard IPC messages
