#include "cpu/isr.h"
#include "libkern/stdio.h"

Isr interrupt_handlers[256];
void register_interrupt_handler(uint8_t number, Isr handler) {
    interrupt_handlers[number] = handler;
}

void isr_handler(Registers regs) {
    kprintf(LOG_INFO, "Received: %d\n", regs.interrupt_number);
}

void irq_handler(Registers regs) {
    /* Send an EOI (end of interrupt) signal to the PIC */
    if (regs.interrupt_number >= 40)
        outb(0xA0, 0x20);

    outb(0x20, 0x20);

    if (interrupt_handlers[regs.interrupt_number] != 0) {
        Isr handler = interrupt_handlers[regs.interrupt_number];
        isr_handler(regs);
    }
}
