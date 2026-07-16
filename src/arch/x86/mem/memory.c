/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "arch/x86/mem.h"
#include "arch/x86/vmm.h"
#include "arch/x86/heap.h"
#include "kernel/qemu.h"

extern uint8_t _kernel_end;

void memory_initialize(struct multiboot_info *mbi) {
    uint32_t physical_alloc_start = ((uint32_t)&_kernel_end + 0xFFFU) & ~0xFFFU;

    /* Check if multiboot modules exist, and adjust start pointer past them */
    if ((mbi->flags & (1U << 3)) && mbi->mods_count > 0) {
        uint32_t first_mod_end = *(uint32_t *)(mbi->mods_addr + 4);
        uint32_t aligned_mod_end = (first_mod_end + 0xFFFU) & ~0xFFFU;

        if (aligned_mod_end > physical_alloc_start)
            physical_alloc_start = aligned_mod_end;
    }

    /* Determine upper memory bounds */
    uint64_t mem_high_point = 0;
    if (mbi->flags & (1U << 0))
        mem_high_point = (uint64_t)mbi->mem_upper * 1024;

    if (mem_high_point > MAX_PHYSICAL_BYTES || mem_high_point == 0)
        mem_high_point = MAX_PHYSICAL_BYTES;

    qemu_printf(QEMU_MEM, QEMU_INFO, "Memory map: (physical allocation start: 0x%x, memory high point: 0x%x)",
                physical_alloc_start, (uint32_t)mem_high_point);

    /* Initialize VMM & PMM */
    uint32_t post_paging_free_mem = vmm_initialize((uint32_t)mem_high_point, physical_alloc_start);

    /* Initialize Heap */
    uint32_t heap_size = 1024 * 1024 * 4; /* 4MB initial size */
    heap_initialize((void *)post_paging_free_mem, heap_size);
}
