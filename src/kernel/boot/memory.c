/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "kernel/multiboot.h"

#if defined(__i386__) || defined(_M_IX86)
#include "arch/x86/mem.h"
#else
#error "Unsupported architecture! (i386 is available)"
#endif

#include "kernel/core/mem.h"

/**
 * Get the virtual address of the multiboot info structure.
 * Converts physical address to virtual if necessary.
 */
struct multiboot_info *boot_init_get_mbi_virtual(struct multiboot_info *mbi) {
    if ((uintptr_t)mbi < KERNEL_VIRTUAL_OFFSET)
        return (struct multiboot_info *)ADD_KERNEL_OFFSET((uintptr_t)mbi);
    return mbi;
}
