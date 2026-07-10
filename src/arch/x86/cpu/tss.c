#include "arch/x86/tss.h"
#include "arch/x86/gdt.h"
#include "libkern/memory.h"
#include "kernel/qemu.h"

struct tss_entry tss;

void write_tss(uint16_t ss0, uint32_t esp0) {
    qemu_printf(QEMU_INFO, "Writing TSS");
    uint32_t base = (uint32_t)&tss;
    uint32_t limit = base + sizeof(struct tss_entry);

    gdt_set_gate(5, base, limit, 0xE9, 0x0);
    memset(&tss, 0, sizeof(struct tss_entry));

    tss.ss0 = ss0;
    tss.esp0 = esp0;
    tss.cs = 0x08 | 0x3;
    tss.ss = tss.ds = tss.es = tss.fs = tss.gs = 0x10 | 0x3;
}
