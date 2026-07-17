/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "arch/x86/features.h"
#include "kernel/qemu.h"

void initialize_cpu_features(void) {
    /* Enable legacy x87 fpu */
    uintptr_t cr0;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));

    cr0 &= ~(1 << 2); /* Clear emulation bit */
    cr0 &= ~(1 << 3); /* Clear task switched bit */
    cr0 |= (1 << 1);  /* Set monitor coprocessor bit, monitor fpu instructions */
    cr0 |= (1 << 5);  /* Set numeric error bit, enable standard floating point errors */

    __asm__ volatile("mov %0, %%cr0" ::"r"(cr0));

    /* Initialize the fpt state */
    __asm__ volatile("finit");

    /* Enable sse */
    uintptr_t cr4;
    __asm__ volatile("mov %%cr4, %0" : "=r"(cr4));

    cr4 |= (1 << 9);  /* Set osfxsr, support fxsave/fxrstor for sse state */
    cr4 |= (1 << 10); /* Set osxmmexcpt, support simd exceptions */

    __asm__ volatile("mov %0, %%cr4" ::"r"(cr4));

    qemu_printf(QEMU_CPU, QEMU_OK, "CPU features initialized (cr0: 0x%x, cr4: 0x%x)", cr0, cr4);
}
