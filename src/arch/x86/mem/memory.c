/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "arch/x86/heap.h"
#include "arch/x86/mem.h"
#include "arch/x86/vmm.h"
#include "kernel/qemu.h"

extern uint32_t _kernel_start;
extern uint32_t _kernel_end;

void memory_initialize(struct multiboot_info *mbi) {
    auto virtual_alloc_start = ((uint32_t)&_kernel_end + 0x0000'0FFFU) & ~0x0000'0FFFU;
    auto physical_alloc_start = virtual_alloc_start - (uint32_t)&_kernel_start;

    /* Check if multiboot modules exist, and adjust start pointer past them */
    if ((mbi->flags & (1U << 3)) && mbi->mods_count > 0) {
        auto first_mod_end = *(uint32_t *)(mbi->mods_addr + 4);
        auto aligned_mod_end = (first_mod_end + 0x0000'0FFFU) & ~0x0000'0FFFU;

        if (aligned_mod_end > physical_alloc_start)
            physical_alloc_start = aligned_mod_end;
    }

    /* Determine upper memory bounds */
    uint64_t mem_high_point = 0;
    if (mbi->flags & (1U << 0))
        mem_high_point = (uint64_t)mbi->mem_upper * 1024U;

    if (mem_high_point > MAX_PHYSICAL_BYTES || mem_high_point == 0)
        mem_high_point = MAX_PHYSICAL_BYTES;

    qemu_printf(QEMU_MEM, QEMU_INFO, "Memory map: (physical allocation start: 0x%x, memory high point: 0x%x)",
                physical_alloc_start, (uint32_t)mem_high_point);

    /* Initialize VMM & PMM */
    auto post_paging_free_mem_phys = vmm_initialize((uint32_t)mem_high_point, physical_alloc_start);

    /* Initialize Heap */
    constexpr uint32_t heap_size = 4'194'304; /* 4MB initial size */
    auto post_paging_free_mem_virt = post_paging_free_mem_phys + (uint32_t)&_kernel_start;

    heap_initialize((void *)post_paging_free_mem_virt, heap_size);
}
