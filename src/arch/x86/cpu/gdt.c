/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "arch/x86/gdt.h"

#include "arch/x86/tss.h"
#include "kernel/interrupts.h"
#include "kernel/qemu.h"

static struct gdt_entry entries[6];
static struct gdt_pointer ptr;

extern void gdt_flush(uint32_t);
extern void tss_flush(void);

void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    entries[num].base_low = (base & 0xFFFF);
    entries[num].base_middle = (base >> 16) & 0xFF;
    entries[num].base_high = (base >> 24) & 0xFF;

    entries[num].limit_low = (limit & 0xFFFF);
    entries[num].gran = (limit >> 16) & 0x0F;
    entries[num].gran |= gran & 0xF0;
    entries[num].access = access;
}

void gdt_initialize() {
    qemu_printf(QEMU_INFO, "Initializing GDT");

    ptr.base = (uint32_t)&entries;
    ptr.limit = (uint16_t)sizeof(entries) - 1;

    disable_interrupts();

    qemu_printf(QEMU_INFO, "Setting GDT gates");
    gdt_set_gate(0, 0, 0, 0, 0);
    gdt_set_gate(1, 0, 0xFFFFF, 0x9A, 0xCF);
    gdt_set_gate(2, 0, 0xFFFFF, 0x92, 0xCF);
    gdt_set_gate(3, 0, 0xFFFFF, 0xFA, 0xCF);
    gdt_set_gate(4, 0, 0xFFFFF, 0xF2, 0xCF);
    write_tss(0x10, 0x0);

    qemu_printf(QEMU_INFO, "Flushing GDT & TSS");
    gdt_flush((uint32_t)&ptr);
    tss_flush();
}
