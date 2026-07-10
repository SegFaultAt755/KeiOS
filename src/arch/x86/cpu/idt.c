#include "arch/x86/idt.h"
#include "libkern/stdio.h"
#include "libkern/memory.h"
#include "kernel/qemu.h"
#include <stddef.h>

static struct idt_gate entries[256];
static struct idt_pointer ptr;

void idt_set_gate(uint8_t vec, uint32_t isr, uint8_t attribs) {
    entries[vec].offset_low = isr & 0xFFFF;
    entries[vec].offset_high = (isr >> 16) & 0xFFFF;
    entries[vec].attribs = attribs | 0x60;

    entries[vec].segment_sel = 0x08;
    entries[vec].reserved = 0;
}

void remap_irq(void) {
    qemu_printf(QEMU_INFO, "Remapping IRQ");
    struct pic {
        uint16_t port;
        uint8_t val;
    };

    static const struct pic seq[] = {
        {0x20, 0x11}, {0xA0, 0x11},
        {0x21, 0x20}, {0xA1, 0x28},
        {0x21, 0x04}, {0xA1, 0x02},
        {0x21, 0x01}, {0xA1, 0x01},
        {0x21, 0x00}, {0xA1, 0x00}
    };

    for (size_t i = 0; i < sizeof(seq) / sizeof(seq[0]); i++) {
        outb(seq[i].port, seq[i].val);
        outb(0x80, 0);
    }
}

void setup_irq(void) {
    qemu_printf(QEMU_INFO, "Setting up IRQ");

    static const void* handlers[] = {
        irq0, irq1, irq2, irq3, irq4, irq5, irq6, irq7,
        irq8, irq9, irq10, irq11, irq12, irq13, irq14, irq15
    };

    for (int i = 0; i < 16; i++)
        idt_set_gate(i + 32, (uint32_t)handlers[i], 0x8E);
}

void setup_idt() {
    qemu_printf(QEMU_INFO, "Setting up IDT");

    static const void* handlers[] = {
        isr0, isr1, isr2, isr3, isr4, isr5, isr6, isr7,
        isr8, isr9, isr10, isr11, isr12, isr13, isr14, isr15,
        isr16, isr17, isr18, isr19, isr20, isr21, isr22, isr23,
        isr24, isr25, isr26, isr27, isr28, isr29, isr30, isr31
    };

    for (int i = 0; i < 32; i++)
        idt_set_gate(i, (uint32_t)handlers[i], 0x8E);
}

void idt_initialize(void) {
    qemu_printf(QEMU_INFO, "Initializing IDT");

    ptr.base = (uint32_t)&entries[0];
    ptr.limit = (uint16_t)sizeof(entries) - 1;
    memset(&entries, 0, sizeof(struct idt_gate) * 256);

    setup_idt();
    remap_irq();
    setup_irq();

    __asm__ volatile ("lidt %0" : : "m" (ptr));
}
