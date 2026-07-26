/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "arch/x86/gdt.h"

#include "arch/x86/tss.h"
#include "kernel/interrupts.h"
#include "kernel/qemu.h"

constexpr size_t GDT_ENTRIES_COUNT = 6;

static struct gdt_entry entries[GDT_ENTRIES_COUNT];
static struct gdt_pointer ptr;

extern void gdt_flush(uint32_t);
extern void tss_flush();

void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    entries[num].base_low = (base & 0xFFFFU);
    entries[num].base_middle = (base >> 16) & 0xFFU;
    entries[num].base_high = (base >> 24) & 0xFFU;

    entries[num].limit_low = (limit & 0xFFFFU);
    entries[num].gran = (limit >> 16) & 0x0FU;
    entries[num].gran |= gran & 0xF0U;
    entries[num].access = access;
}

void gdt_initialize() {
    ptr.base = (uint32_t)&entries;
    ptr.limit = (uint16_t)sizeof(entries) - 1;

    disable_interrupts();

    gdt_set_gate(0, 0, 0, 0, 0);
    gdt_set_gate(1, 0, 0x000F'FFFFU, 0x9A, 0xCF);
    gdt_set_gate(2, 0, 0x000F'FFFFU, 0x92, 0xCF);
    gdt_set_gate(3, 0, 0x000F'FFFFU, 0xFA, 0xCF);
    gdt_set_gate(4, 0, 0x000F'FFFFU, 0xF2, 0xCF);
    write_tss(0x10, 0x0);

    gdt_flush((uint32_t)&ptr);
    tss_flush();

    qemu_printf(QEMU_CPU, QEMU_OK, "GDT & TSS loaded");
}
