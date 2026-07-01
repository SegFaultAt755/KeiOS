#include "cpu/idt.h"
#include "libkern/stdio.h"
#include "libkern/memory.h"

GateDescriptor idt_entries[256];
Idtr idtr;

void idt_set_gate(uint8_t vector, uint32_t isr, uint8_t attributes) {
    idt_entries[vector].offset_low = isr & 0xFFFF;
    idt_entries[vector].offset_high = (isr >> 16) & 0xFFFF;
    idt_entries[vector].attributes = attributes | 0x60;

    idt_entries[vector].segment_selector = 0x08;
    idt_entries[vector].reserved = 0;
}

void remap_irq(void) {
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0x0);
    outb(0xA1, 0x0);
}

void setup_irq(void) {
    idt_set_gate(32, IRQ0, 0x8E);
    idt_set_gate(33, IRQ1, 0x8E);
    idt_set_gate(34, IRQ2, 0x8E);
    idt_set_gate(35, IRQ3, 0x8E);
    idt_set_gate(36, IRQ4, 0x8E);
    idt_set_gate(37, IRQ5, 0x8E);
    idt_set_gate(38, IRQ6, 0x8E);
    idt_set_gate(39, IRQ7, 0x8E);
    idt_set_gate(40, IRQ8, 0x8E);
    idt_set_gate(41, IRQ9, 0x8E);
    idt_set_gate(42, IRQ10, 0x8E);
    idt_set_gate(43, IRQ11, 0x8E);
    idt_set_gate(44, IRQ12, 0x8E);
    idt_set_gate(45, IRQ13, 0x8E);
    idt_set_gate(46, IRQ14, 0x8E);
    idt_set_gate(47, IRQ15, 0x8E);
}

void setup_idt() {
    idt_set_gate(0, (uint32_t) isr0, 0x8E);
    idt_set_gate(1, (uint32_t) isr1, 0x8E);
    idt_set_gate(2, (uint32_t) isr2, 0x8E);
    idt_set_gate(3, (uint32_t) isr3, 0x8E);
    idt_set_gate(4, (uint32_t) isr4, 0x8E);
    idt_set_gate(5, (uint32_t) isr5, 0x8E);
    idt_set_gate(6, (uint32_t) isr6, 0x8E);
    idt_set_gate(7, (uint32_t) isr7, 0x8E);
    idt_set_gate(8, (uint32_t) isr8, 0x8E);
    idt_set_gate(9, (uint32_t) isr9, 0x8E);
    idt_set_gate(10, (uint32_t) isr10, 0x8E);
    idt_set_gate(11, (uint32_t) isr11, 0x8E);
    idt_set_gate(12, (uint32_t) isr12, 0x8E);
    idt_set_gate(13, (uint32_t) isr13, 0x8E);
    idt_set_gate(14, (uint32_t) isr14, 0x8E);
    idt_set_gate(15, (uint32_t) isr15, 0x8E);
    idt_set_gate(16, (uint32_t) isr16, 0x8E);
    idt_set_gate(17, (uint32_t) isr17, 0x8E);
    idt_set_gate(18, (uint32_t) isr18, 0x8E);
    idt_set_gate(19, (uint32_t) isr19, 0x8E);
    idt_set_gate(20, (uint32_t) isr20, 0x8E);
    idt_set_gate(21, (uint32_t) isr21, 0x8E);
    idt_set_gate(22, (uint32_t) isr22, 0x8E);
    idt_set_gate(23, (uint32_t) isr23, 0x8E);
    idt_set_gate(24, (uint32_t) isr24, 0x8E);
    idt_set_gate(25, (uint32_t) isr25, 0x8E);
    idt_set_gate(26, (uint32_t) isr26, 0x8E);
    idt_set_gate(27, (uint32_t) isr27, 0x8E);
    idt_set_gate(28, (uint32_t) isr28, 0x8E);
    idt_set_gate(29, (uint32_t) isr29, 0x8E);
    idt_set_gate(30, (uint32_t) isr30, 0x8E);
    idt_set_gate(31, (uint32_t) isr31, 0x8E);
}

void idt_initialize(void) {
    idtr.offset = (uint32_t) &idt_entries[0];
    idtr.size = (uint16_t) sizeof(GateDescriptor) * 256 - 1;
    memset(&idt_entries, 0, sizeof(GateDescriptor) * 256);

    setup_idt();
    remap_irq();
    setup_irq();

    __asm__ volatile ("lidt %0" : : "m" (idtr));
}