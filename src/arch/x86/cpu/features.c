/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "arch/x86/features.h"
#include "kernel/qemu.h"

void cpu_feat_init() {
    /* Enable legacy x87 fpu */
    uintptr_t cr0_val;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0_val));

    auto cr0 = cr0_val;
    cr0 &= ~(1U << 2); /* Clear emulation bit */
    cr0 &= ~(1U << 3); /* Clear task switched bit */
    cr0 |= (1U << 1);  /* Set monitor coprocessor bit, monitor fpu instructions */
    cr0 |= (1U << 5);  /* Set numeric error bit, enable standard floating point errors */

    __asm__ volatile("mov %0, %%cr0" ::"r"(cr0));

    /* Initialize the fpt state */
    __asm__ volatile("finit");

    /* Enable sse */
    uintptr_t cr4_val;
    __asm__ volatile("mov %%cr4, %0" : "=r"(cr4_val));

    auto cr4 = cr4_val;
    cr4 |= (1U << 9);  /* Set osfxsr, support fxsave/fxrstor for sse state */
    cr4 |= (1U << 10); /* Set osxmmexcpt, support simd exceptions */

    __asm__ volatile("mov %0, %%cr4" ::"r"(cr4));

    qemu_printf(QEMU_CPU, QEMU_OK, "CPU features initialized (cr0: %p, cr4: %p)", cr0, cr4);
}
