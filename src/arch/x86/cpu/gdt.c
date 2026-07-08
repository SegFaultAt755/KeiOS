#include "arch/x86/gdt.h"
#include "arch/x86/tss.h"
#include "kernel/interrupts.h"
#include "kernel/qemu.h"

GdtEntry gdt_entries[6];
GdtPointer gdt_ptr;

extern void gdt_flush(uint32_t);
extern void tss_flush(void);

void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt_entries[num].base_low = (base & 0xFFFF);
    gdt_entries[num].base_middle = (base >> 16) & 0xFF;
    gdt_entries[num].base_high = (base >> 24) & 0xFF;

    gdt_entries[num].limit_low = (limit & 0xFFFF);
    gdt_entries[num].granularity = (limit >> 16) & 0x0F;
    gdt_entries[num].granularity |= gran & 0xF0;
    gdt_entries[num].access = access;
}

void gdt_initialize() {
    qemu_printf(QEMU_LOG_INFO, "Initializing GDT");

    gdt_ptr.limit = sizeof(gdt_entries) - 1;
    gdt_ptr.base  = (uint32_t) &gdt_entries;

    disable_interrupts();

    qemu_printf(QEMU_LOG_INFO, "Setting GDT gates");
    gdt_set_gate(0, 0, 0, 0, 0); /* Null descriptor */

    gdt_set_gate(1, 0, 0xFFFFF, 0x9A, 0xCF); /* Kernel code segment */
    gdt_set_gate(2, 0, 0xFFFFF, 0x92, 0xCF); /* Kernel data segment */
    gdt_set_gate(3, 0, 0xFFFFF, 0xFA, 0xCF); /* User code segment */
    gdt_set_gate(4, 0, 0xFFFFF, 0xF2, 0xCF); /* User data segment */
    write_tss(0x10, 0x0); /* Task state segment */

    qemu_printf(QEMU_LOG_INFO, "Flushing GDT & TSS");
    gdt_flush((uint32_t) &gdt_ptr);
    tss_flush();
}
