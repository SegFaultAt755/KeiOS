#include "arch/x86/isr.h"
#include "libkern/stdio.h"

void irq_handler(Registers *regs) {
    /* Send an EOI (end of interrupt) signal to the PIC */
    if (regs -> interrupt_number >= 40) {
        outb(0xA0, 0x20);
        outb(0x80, 0);
    }

    /* Send the job to master */
    outb(0x20, 0x20);
    outb(0x80, 0);

    if (interrupt_handlers[regs -> interrupt_number] != 0) {
        Isr handler = interrupt_handlers[regs -> interrupt_number];
        handler(regs);
    }
}
