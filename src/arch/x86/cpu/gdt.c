/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "arch/x86/gdt.h"

#include "arch/x86/tss.h"
#include "kernel/interrupts.h"
#include "kernel/qemu.h"

constexpr size_t GDT_ENTRY_COUNT = 6;
uint8_t kernel_stack[8192];

static struct gdt_entry gdt_entries[GDT_ENTRY_COUNT];
static struct gdt_pointer gdt_ptr;

extern void gdt_flush(uint32_t);
extern void tss_flush();

void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt_entries[num].base_low = (base & 0xFFFFU);
    gdt_entries[num].base_middle = (base >> 16) & 0xFFU;
    gdt_entries[num].base_high = (base >> 24) & 0xFFU;

    gdt_entries[num].limit_low = (limit & 0xFFFFU);
    gdt_entries[num].gran = (limit >> 16) & 0x0FU;
    gdt_entries[num].gran |= gran & 0xF0U;
    gdt_entries[num].access = access;
}

void gdt_init() {
    gdt_ptr.base = (uint32_t)&gdt_entries;
    gdt_ptr.limit = (uint16_t)sizeof(gdt_entries) - 1;

    disable_interrupts();

    gdt_set_gate(0, 0, 0, 0, 0);
    gdt_set_gate(1, 0, 0x000F'FFFFU, 0x9A, 0xCF);
    gdt_set_gate(2, 0, 0x000F'FFFFU, 0x92, 0xCF);
    gdt_set_gate(3, 0, 0x000F'FFFFU, 0xFA, 0xCF);
    gdt_set_gate(4, 0, 0x000F'FFFFU, 0xF2, 0xCF);
    write_tss(0x10, (uint32_t)&kernel_stack + 8192);

    gdt_flush((uint32_t)&gdt_ptr);
    tss_flush();

    qemu_printf(QEMU_CPU, QEMU_OK, "GDT & TSS loaded");
}
