#include "drivers/pit.h"
#include "libkern/stdio.h"
#include "kernel/qemu.h"

void pit_initialize(uint32_t freq, void (*callback)(struct registers *regs)) {
    qemu_printf(QEMU_INFO, "Initializing timer : %d", freq);
    intr_handler(IRQ0, callback);

    uint32_t divisor = 119'318'0 / freq;
    outb(0x43, 0x36);

    uint8_t l = (uint8_t)(divisor & 0xFF);
    uint8_t h = (uint8_t)((divisor >> 8) & 0xFF);

    /* Send the frequency divisor */
    qemu_printf(QEMU_INFO, "Outcoming frequency {%d:%d}", l, h);
    outb(0x40, l); outb(0x40, h);
}