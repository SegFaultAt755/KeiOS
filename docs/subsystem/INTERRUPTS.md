# Subsystem Deep-Dive into Interrupts & CPU Exceptions

---
## Overview & Architecture

The interrupt and exception handling subsystem routes hardware IRQs, CPU fault traps, and software interrupts (system calls) through an organized control path

Execution flow from hardware trigger to service handler:

`````txt
+-------------------------------------------------------------------+
| Hardware Event / CPU Trap                                         |
+-------------------------------------------------------------------+
    |
    v
[Programmable PIC] -----------> Remaps hardware interrupts to IRQ 32-47
[Interrupt Descriptor Table] -> Points to assembly stub routines (ISRs)
[Assembly Interrupt Stub] ----> Pushes register frame & switches CPU contexts
[C Interrupt Handler] --------> Routes interrupt to microkernel driver / subsystem
`````

---
## Subsystem Status

| **Module**                | **Level**                | **Status**     | **Description**                                   |
| ------------------------- | ------------------------ | -------------- | ------------------------------------------------- |
| **GDT & TSS**             | Privilege/Segment        | 🟢 Completed   | Segment setup, Task State Segment stack switching |
| **IDT & Exception Traps** | Low Level CPU            | 🟢 Completed   | Catching Faults (`#PF`, `#GP`, `#DB`, etc.)       |
| **PIC Remapping**         | Hardware Controller      | 🟢 Completed   | Mapping IRQ 0-15 out of CPU reserved vector range |
| **System Call Entry**     | Inter Ring Communication | 🟡 In Progress | Software Interrupt `int 0x80` handling            |

---
## Global Descriptor Table (GDT) & TSS `[COMPLETED]`

Although x86 32-bit paging handles primary memory protection, the x86 hardware architecture mandates standard GDT segment setup for Ring 0 and Ring 3 boundary transitions

### Segment Descriptor Layout

| **Selector** | **Segment Name**    | **Base Address** | **Limit**     | **DPL** | **Usage**                     |
| ------------ | ------------------- | ---------------- | ------------- | ------- | ----------------------------- |
| `0x00`       | Null Descriptor     | `0x00000000`     | `0x00000000`  | None    | Required by x86 specification |
| `0x08`       | Kernel Code Segment | `0x00000000`     | `0xFFFFFFFF`  | Ring 0  | Microkernel Code Execution    |
| `0x10`       | Kernel Data Segment | `0x00000000`     | `0xFFFFFFFF`  | Ring 0  | Microkernel Stack & Heap Data |
| `0x18`       | User Code Segment   | `0x00000000`     | `0xFFFFFFFF`  | Ring 3  | Ring 3 User Apps / Drivers    |
| `0x20`       | User Data Segment   | `0x00000000`     | `0xFFFFFFFF`  | Ring 3  | Ring 3 User Stack & Memory    |
| `0x28`       | Task State Segment  | `&tss_entry`     | `sizeof(tss)` | Ring 3  | Holds Kernel Stack `ESP0`     |

_**Crucial Architecture Note:** The Task State Segment (TSS) is **required** by x86 hardware when transitioning from Ring 3 to Ring 0 during an interrupt. The CPU reads `tss.esp0` to load the target kernel stack before pushing user register frames_

---
## Programmable Interrupt Controller (PIC) `[COMPLETED]`

The dual 8259 PICs are remapped during early kernel initialization to prevent hardware interrupts from conflicting with standard x86 CPU Exception Vectors (0–31)

`````txt
+------------------+-------------------+--------------------+
| Controller       | Hardware IRQs     | Remapped Vector    |
+------------------+-------------------+--------------------+
| Master PIC       | IRQ 0 - IRQ 7     | Interrupt 32 - 39  |
| Slave PIC        | IRQ 8 - IRQ 15    | Interrupt 40 - 47  |
+------------------+-------------------+--------------------+
`````

### Key IRQ Routing

- **Vector 32 (IRQ 0):** Programmable Interval Timer (PIT) -> Preemptive Scheduler
- **Vector 33 (IRQ 1):** PS/2 Keyboard Controller -> Ring 3 Driver IPC Event
- **Vector 128 (`0x80`):** Software System Call Vector -> Kernel IPC & Syscall Handler

---
## Software System Call Gate (`int 0x80`) `[IN PROGRESS]`

Applications and driver servers invoke kernel services using vector `0x80`. The gate descriptor in the IDT is explicitly initialized with Descriptor Privilege Level (**DPL 3**), allowing Ring 3 processes to trigger it without generating a `#GP` (General Protection Fault)

### System Call Register Protocol

- **`eax`:** System Call Identifier
- **`ebx`:** Parameter 1
- **`ecx`:** Parameter 2
- **`edx`:** Parameter 3
- **`eax` (Return):** Status Code / Result Payload
