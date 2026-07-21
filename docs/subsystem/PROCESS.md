# Subsystem Deep-Dive into Task Management & Processes

---
## Overview & Architecture

The process and scheduling subsystem provides isolation between executing processes and enables controlled execution transitions from Ring 0 to Ring 3

`````txt
+-------------------------------------------------------------------+
| USER SPACE (Ring 3)                                               |
|                                                                   |
|  +---------------------------+     +---------------------------+  |
|  | User Task A (Display)     |     | User Task B (Shell)       |  |
|  | - Private Virtual Address |     | - Private Virtual Address |  |
|  | - Dedicated Ring 3 Stack  |     | - Dedicated Ring 3 Stack  |  |
|  +-------------+-------------+     +-------------+-------------+  |
+----------------|---------------------------------|----------------+
|                |  int 0x80 / Timer IRQ           |                |
|                v                                 v                |
+-------------------------------------------------------------------+
| KERNEL SPACE (Ring 0)                                             |
|                                                                   |
|  +-------------------------------------------------------------+  |
|  | Preemptive Scheduler & Context Switcher                     |  |
|  | - Restores CPU Trap Frames                                  |  |
|  | - Switches Page Directory CR3                               |  |
|  | - Updates TSS esp0 Kernel Stack Pointer                     |  |
|  +-------------------------------------------------------------+  |
+-------------------------------------------------------------------+
`````

---
## Subsystem Status

| **Module**                | **Level**        | **Status**     | **Description**                               |
| ------------------------- | ---------------- | -------------- | --------------------------------------------- |
| **ELF32 Parser**          | Program Loader   | 🟡 In Progress | Loading segment headers from `initrd` tarball |
| **Ring 3 Transition**     | Task Abstraction | 🟡 In Progress | Stack frame setup for `IRET` drop to Ring 3   |
| **Process Control Block** | Task Abstraction | 🟡 In Progress | Context structures and status handling        |
| **Preemptive Scheduler**  | Task Management  | 🔴 Planned     | Timer driven round-robin context switching    |

---
## Process Control Block (PCB)

Each process in the system is represented by a `task` structure tracking memory mappings, kernel stack frames, execution state, and privilege information

`````c
enum task_state {
    TASK_READY,
    TASK_RUNNING,
    TASK_BLOCKED,
    TASK_TERMINATED
};

struct task {
    uint32_t pid;             /* Unique process ID */
    task_state state;         /* Current execution state */
    page_dir_t *page_dir;     /* Physical address of task cr3 directory */
    uint32_t kernel_stack;    /* Base pointer to kernel stack (tss esp0) */
    uint32_t user_stack;      /* Base pointer to user stack (ring 3) */
    struct registers context; /* Saved registers during context switch */
    struct task *next;        /* Link to next task in scheduler queue */
};
`````

---
## Ring 0 to Ring 3 Privilege Switch (`IRET`) `[IN PROGRESS]`

Because x86 CPUs don't provide a direct instruction to lower execution privileges from Ring 0 to Ring 3, the microkernel manually fabricates an interrupt return stack frame and executes the `IRET` instruction

### Stack Construction Sequence

`````txt
High Memory (Top of Stack)
  +-----------------------+
  | User Data Selector    | SS (0x23: User Data Seg | RPL 3)
  +-----------------------+
  | User Stack Pointer    | ESP (Pointer to Ring 3 Stack)
  +-----------------------+
  | EFLAGS Register       | EFLAGS (Interrupt Flag IF bit set)
  +-----------------------+
  | User Code Selector    | CS (0x1B: User Code Seg | RPL 3)
  +-----------------------+
  | Instruction Pointer   | EIP (Entry Point of ELF Program)
  +-----------------------+
Low Memory (Stack Pointer ESP)
`````

1. Kernel constructs this fake interrupt frame on the task **Kernel Stack**
2. Set `tss.esp0` to point to the top of the task kernel stack (so subsequent interrupts safely switch back to Ring 0)
3. Load the task Page Directory physical address into register `CR3`
4. Execute `IRET`: CPU pops `EIP`, `CS`, `EFLAGS`, `ESP`, and `SS`, simultaneously lowering CPU privilege to **Ring 3**

---
## Driver Isolation & Permission Overrides `[PLANNED]`

Drivers (such as the Display Server or PS/2 Controller) execute entirely in Ring 3. When special hardware access is required, the kernel configures per task exceptions during creation:

- **Direct Framebuffer Access:** VMM maps physical Video RAM directly into the process virtual memory range (`0xFD000000`)
- **Hardware Port Access (`inb`/`outb`):** Kernel edits the **I/O Permission Bitmap** inside the task TSS structure to grant access to specific hardware ports without allowing unrestricted Ring 0 rights
