#include "arch/x86/idt.h"
#include "libkern/stdio.h"
#include "libkern/memory.h"
#include "kernel/qemu.h"
#include <stddef.h>

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
    qemu_printf(QEMU_LOG_INFO, "Remapping IRQ");
    typedef struct Pic {
        uint16_t port;
        uint8_t value;
    } Pic;

    static const struct Pic sequence[] = {
        {0x20, 0x11}, {0xA0, 0x11},  /* ICW1: Init command */
        {0x21, 0x20}, {0xA1, 0x28},  /* ICW2: Vector offsets (32 and 40) */
        {0x21, 0x04}, {0xA1, 0x02},  /* ICW3: Cascade wiring info */
        {0x21, 0x01}, {0xA1, 0x01},  /* ICW4: 8086 mode environment */
        {0x21, 0x00}, {0xA1, 0x00}   /* OCW1: Clear masks (Enable all IRQs) */
    };

    const size_t total_steps = sizeof(sequence) / sizeof(sequence[0]);
    for (size_t i = 0; i < total_steps; i++) {
        outb(sequence[i].port, sequence[i].value);
        outb(0x80, 0);
    }
}

void setup_irq(void) {
    qemu_printf(QEMU_LOG_INFO, "Setting up IRQ");

    static const void* irq_handlers[] = {
        irq0, irq1, irq2, irq3, irq4, irq5, irq6, irq7,
        irq8, irq9, irq10, irq11, irq12, irq13, irq14, irq15
    };

    for (int i = 0; i < 16; i++) {
        idt_set_gate(i+32, (uint32_t) irq_handlers[i], 0x8E);
    }
}

void setup_idt() {
    qemu_printf(QEMU_LOG_INFO, "Setting up IDT");

    static const void* isr_handlers[] = {
        isr0, isr1, isr2, isr3, isr4, isr5, isr6, isr7,
        isr8, isr9, isr10, isr11, isr12, isr13, isr14, isr15,
        isr16, isr17, isr18, isr19, isr20, isr21, isr22, isr23,
        isr24, isr25, isr26, isr27, isr28, isr29, isr30, isr31
    };

    for (int i = 0; i < 32; i++) {
        idt_set_gate(i, (uint32_t) isr_handlers[i], 0x8E);
    }
}

void idt_initialize(void) {
    qemu_printf(QEMU_LOG_INFO, "Initializing IDT");

    idtr.offset = (uint32_t) &idt_entries[0];
    idtr.size = (uint16_t) sizeof(idt_entries) - 1;
    memset(&idt_entries, 0, sizeof(GateDescriptor) * 256);

    setup_idt();
    remap_irq();
    setup_irq();

    __asm__ volatile ("lidt %0" : : "m" (idtr));
}
