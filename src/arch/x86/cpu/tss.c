#include "arch/x86/tss.h"
#include "arch/x86/gdt.h"
#include "libkern/memory.h"
#include "kernel/qemu.h"

TssEntry tss_entry;

void write_tss(uint16_t ss0, uint32_t esp0) {
    qemu_printf(QEMU_LOG_INFO, "Writing TSS");
    uint32_t base = (uint32_t) &tss_entry;
    uint32_t limit = base + sizeof(TssEntry);

    gdt_set_gate(5, base, limit, 0xE9, 0x0);
    memset(&tss_entry, 0, sizeof(TssEntry));

    tss_entry.ss0 = ss0;
    tss_entry.esp0 = esp0;
    tss_entry.cs = 0x08 | 0x3;
    
    tss_entry.ss =
    tss_entry.ds =
    tss_entry.es =
    tss_entry.fs =
    tss_entry.gs = 0x10 | 0x3;
}