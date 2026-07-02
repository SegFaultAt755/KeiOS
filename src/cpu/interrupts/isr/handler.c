#include "cpu/isr.h"
#include "libkern/stdio.h"

Isr interrupt_handlers[256];
void register_interrupt_handler(uint8_t number, Isr handler) {
    interrupt_handlers[number] = handler;
}

void isr_handler(Registers regs) {
    /* Do nothing */
}
