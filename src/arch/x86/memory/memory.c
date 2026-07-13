/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "arch/x86/mem.h"
#include "kernel/qemu.h"

extern uint8_t _kernel_end;

void memory_initialize(struct multiboot_info *mbi) {
    qemu_printf(QEMU_INFO, "Initializing memory components");

    uint32_t physical_alloc_start = ((uint32_t)&_kernel_end + 0xFFF) & ~0xFFF;

    /* Check if Multiboot modules exist, and adjust start pointer past them */
    if ((mbi->flags & (1 << 3)) && mbi->mods_count > 0) {
        uint32_t first_mod_end = *(uint32_t *)(mbi->mods_addr + 4);
        uint32_t aligned_mod_end = (first_mod_end + 0xFFF) & ~0xFFF;

        if (aligned_mod_end > physical_alloc_start)
            physical_alloc_start = aligned_mod_end;
    }

    qemu_printf(QEMU_INFO, "Physical allocation starts at: 0x%x", physical_alloc_start);

    /* Determine upper memory bounds */
    uint64_t mem_high_point = 0;
    if (mbi->flags & (1 << 0))
        mem_high_point = (uint64_t)mbi->mem_upper * 1024;

    if (mem_high_point > MAX_PHYSICAL_BYTES || mem_high_point == 0)
        mem_high_point = MAX_PHYSICAL_BYTES;

    qemu_printf(QEMU_INFO, "Memory high point: 0x%x", (uint32_t)mem_high_point);

    /* Initialize Paging */
    uint32_t post_paging_free_mem = paging_initialize((uint32_t)mem_high_point, physical_alloc_start);

    /* Initialize Heap */
    uint32_t heap_size = 1024 * 1024 * 4;
    heap_initialize((void *)post_paging_free_mem, heap_size);
}
