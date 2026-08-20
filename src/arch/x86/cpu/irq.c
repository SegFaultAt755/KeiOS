/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "arch/x86/isr.h"
#include "libkern/stdio.h"

void irq_handler(struct registers *regs) {
    /* Send an end-of-interrupt signal to the PIC */
    if (regs->intr_num >= 40) {
        outb(0xA0, 0x20);
        waitb(1);
    }

    /* Send the interrupt acknowledgment to the master PIC */
    outb(0x20, 0x20);
    waitb(1);

    if (isr_intr_handler[regs->intr_num] != nullptr) {
        auto handler = isr_intr_handler[regs->intr_num];
        handler(regs);
    }
}
