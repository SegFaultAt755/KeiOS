#pragma once

#include <stdint.h>

struct [[gnu::packed]] gate_descriptor {
    uint16_t offset_low;
    uint16_t segment_selector;
    uint8_t  reserved;
    uint8_t  attributes;
    uint16_t offset_high;
};

struct [[gnu::packed]] idt_pointer {
    uint16_t limit;
    uint32_t base;
};

#define ISR_LIST \
    X(0)  X(1)  X(2)  X(3)  X(4)  X(5)  X(6)  X(7)  \
    X(8)  X(9)  X(10) X(11) X(12) X(13) X(14) X(15) \
    X(16) X(17) X(18) X(19) X(20) X(21) X(22) X(23) \
    X(24) X(25) X(26) X(27) X(28) X(29) X(30) X(31)

#define IRQ_LIST \
    X(0)  X(1)  X(2)  X(3)  X(4)  X(5)  X(6)  X(7)  \
    X(8)  X(9)  X(10) X(11) X(12) X(13) X(14) X(15)

#define X(n) extern void isr##n();
    ISR_LIST
#undef X

#define X(n) extern void irq##n();
    IRQ_LIST
#undef X

void idt_set_gate(uint8_t vector, uint32_t isr, uint8_t attributes);
void idt_initialize(void);
