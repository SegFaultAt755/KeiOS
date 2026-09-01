/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

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
    gdt_init();
    idt_init();
    cpu_feat_init();
}
