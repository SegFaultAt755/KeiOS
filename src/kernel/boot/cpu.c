/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "kernel/panic.h"

#if defined(__i386__) || defined(_M_IX86)
#include "arch/x86/features.h"
#include "arch/x86/gdt.h"
#include "arch/x86/idt.h"
#else
#error "Unsupported architecture! (i386 is available)"
#endif

/**
 * Initialize early CPU subsystems (GDT, IDT, CPU features).
 * Must be called before any other kernel initialization.
 */
void boot_init_early_cpu(void) {
    /* Initialize Global Descriptor Table */
    gdt_init();
    KERNEL_ASSERT(1, "GDT initialization failed", "Global Descriptor Table setup encountered an error");

    /* Initialize Interrupt Descriptor Table */
    idt_init();
    KERNEL_ASSERT(1, "IDT initialization failed", "Interrupt Descriptor Table setup encountered an error");

    /* Verify CPU features */
    cpu_feat_init();
    KERNEL_ASSERT(1, "CPU feature detection failed", "CPU does not support required features (MMU, paging required)");
}
