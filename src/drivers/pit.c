/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "drivers/pit.h"

#include "kernel/qemu.h"
#include "libkern/stdio.h"

void pit_initialize(uint32_t freq, void (*callback)(struct registers *regs)) {
    intr_handler(IRQ0, callback);

    uint32_t divisor = 1'193'180 / freq;
    outb(0x43, 0x36);

    uint8_t l = (uint8_t)(divisor & 0xFF);
    uint8_t h = (uint8_t)((divisor >> 8) & 0xFF);

    /* Send the frequency divisor */
    outb(0x40, l);
    outb(0x40, h);

    qemu_printf(QEMU_DRV, QEMU_OK, "PIT configured to %d hz", freq);
}
