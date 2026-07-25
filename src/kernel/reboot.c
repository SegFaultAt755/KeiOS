/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "kernel/reboot.h"

void reboot() {
    /* Gracefully shutdown components */
    /* None yet */

    reboot_raw();
}

void reboot_raw() {
    /* Try via keyboard controller */
    outb(0x64, 0xFE);

    /* Try via PCI register */
    outb(0xCF9, 0x0E);

    /* Perform a warm restart (preserves memory and cache) */
    /* outb(0xCF9, 0x06); */

    /* Try via fast reset */
    uint8_t val = inb(0x92);
    outb(0x92, val | 0x01);

    /* Final option */
    struct [[gnu::packed]] {
        uint16_t limit;
        uint32_t base;
    } null_idt = {0, 0};

    __asm__ volatile("lidt %0; int3" : : "m"(null_idt));
}