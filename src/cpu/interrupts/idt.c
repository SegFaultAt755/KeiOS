#include "cpu/idt.h"

extern void *isr_stub_table[];
GateDescriptor idt[IDT_MAX_DESCRIPTORS];
Idtr idtr;

void idt_set_descriptor(uint8_t vector, void *isr, uint8_t flags) {
    GateDescriptor *descriptor = &idt[vector];

    descriptor -> offset_low = (uint32_t) isr & 0xFFFF;
    descriptor -> segment_selector = 0x08;
    descriptor -> reserved = 0;
    descriptor -> attributes = flags;
    descriptor -> offset_high = ((uint32_t) isr >> 16) & 0xFFFF;
}

void idt_initialize(void) {
    idtr.offset = (uint32_t) &idt[0];
    idtr.size = (uint16_t) sizeof(GateDescriptor) * IDT_MAX_DESCRIPTORS - 1;

    for (uint8_t vector = 0; vector < 32; vector++) {
        idt_set_descriptor(vector, isr_stub_table[vector], 0x8E);
    }

    __asm__ volatile ("lidt %0" : : "m" (idtr));
}