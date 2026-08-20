/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "kernel/reboot.h"
#include "kernel/halt.h"
#include "kernel/interrupts.h"

[[noreturn]] void reboot() {
    /* Shut down available components gracefully */
    /* No component shutdown is needed yet */

    reboot_raw();
}

[[noreturn]] void reboot_raw() {
    /* Try to reboot through the keyboard controller */
    outb(0x64, 0xFE);

    /* Try to reboot through the PCI reset register */
    outb(0xCF9, 0x0E);

    /* Try the fast reset method */
    auto val = inb(0x92);
    outb(0x92, val | 0x01);

    /* Use the final reset method */
    struct [[gnu::packed]] {
        uint16_t limit;
        uint32_t base;
    } null_idt = {0, 0};

    __asm__ volatile("lidt %0; int3" : : "m"(null_idt));

    while (true) {
        disable_interrupts();
        halt();
    }
}
