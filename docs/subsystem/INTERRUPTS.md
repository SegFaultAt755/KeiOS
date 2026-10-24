# Interrupts and Exceptions

## Current implementation

The x86 interrupt setup initializes a GDT and IDT, remaps the legacy dual 8259
PIC to vectors 32–47, installs IRQ stubs, and installs a software syscall gate
at vector `0x80`. CPU exception and IRQ handling is kernel code; there are no
isolated Ring 3 drivers.

The intended hardware IRQ assignments include IRQ 0 for the PIT timer and
IRQ 1 for the PS/2 keyboard. `int 0x80` dispatches to syscall indices documented
in [SYSCALL.md](./SYSCALL.md).

## Current status

| Component | Current status |
| --- | --- |
| GDT and TSS setup | Present |
| IDT and exception stubs | Present; the exception frame ABI and page-fault error-code handling were corrected |
| PIC remapping and IRQ stubs | Present; IRQ privilege attributes are kernel-only |
| PIT and keyboard IRQ paths | Used by the current kernel |
| Syscall gate and dispatcher | Present; only a small kernel-side table is implemented |
| Safe exception recovery and user-mode protection | Not established |

## Known correctness concerns

The IDT now keeps exception and IRQ gates at kernel privilege level while the
`int 0x80` syscall gate remains DPL 3. Full user-mode isolation and safe
exception recovery still need explicit validation before untrusted Ring 3 code
is enabled.
