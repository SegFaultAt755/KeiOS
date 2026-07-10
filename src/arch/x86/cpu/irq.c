#include "arch/x86/isr.h"
#include "libkern/stdio.h"

void irq_handler(struct registers *regs) {
    /* Send an EOI (end of interrupt) signal to the PIC */
    if (regs->intr_num >= 40) {
        outb(0xA0, 0x20);
        waitb(1);
    }

    /* Send the job to master */
    outb(0x20, 0x20);
    waitb(1);

    if (isr_intr_handler[regs->intr_num] != 0) {
        isr_t handler = isr_intr_handler[regs->intr_num];
        handler(regs);
    }
}
