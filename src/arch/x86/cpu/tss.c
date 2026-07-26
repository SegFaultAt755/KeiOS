/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "arch/x86/tss.h"

#include "arch/x86/gdt.h"
#include "kernel/qemu.h"
#include "libkern/memory.h"

static struct tss_entry tss;

void write_tss(uint16_t ss0, uint32_t esp0) {
    auto base = (uint32_t)&tss;
    auto limit = base + sizeof(struct tss_entry);

    gdt_set_gate(5, base, limit, 0xE9, 0x0);
    memset(&tss, 0, sizeof(struct tss_entry));

    tss.ss0 = ss0;
    tss.esp0 = esp0;
    tss.cs = 0x08U | 0x3U;
    tss.ss = tss.ds = tss.es = tss.fs = tss.gs = 0x10U | 0x3U;
}
