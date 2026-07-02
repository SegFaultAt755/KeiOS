#include "drivers/timer.h"
#include "libkern/stdio.h"

void timer_initialize(uint32_t frequency, void (*callback)(Registers *regs)) {
    register_interrupt_handler(IRQ0, callback);

    uint32_t divisor = 1193180 / frequency;
    outb(0x43, 0x36);

    uint8_t l = (uint8_t) (divisor & 0xFF);
    uint8_t h = (uint8_t) ((divisor >> 8) & 0xFF);

    /* Send the frequency divisor */
    outb(0x40, l); outb(0x40, h);
}