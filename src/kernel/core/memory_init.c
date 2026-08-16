/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "kernel/core/memory_init.h"
#include "arch/x86/heap.h"
#include "arch/x86/mem.h"
#include "arch/x86/vmm.h"
#include "kernel/multiboot.h"
#include "kernel/qemu.h"

extern uint32_t _kernel_start;
extern uint32_t _kernel_end;

void memory_initialize(struct multiboot_info *mbi_virtual) {
    auto virtual_alloc_start = ((uint32_t)&_kernel_end + 0x0000'0FFFU) & ~0x0000'0FFFU;
    auto physical_alloc_start = virtual_alloc_start - (uint32_t)&_kernel_start;

    /* Extract the physical MBI */
    uint32_t mbi_phys = (uint32_t)mbi_virtual;
    if (mbi_phys >= KERNEL_VIRTUAL_OFFSET)
        mbi_phys -= KERNEL_VIRTUAL_OFFSET;

    /* Mark physical mbi as reserved */
    uint32_t mbi_end = mbi_phys + sizeof(struct multiboot_info);
    auto aligned_mbi_end = (mbi_end + 0x0000'0FFFU) & ~0x0000'0FFFU;
    
    if (aligned_mbi_end > physical_alloc_start)
        physical_alloc_start = aligned_mbi_end;

    /* Check if multiboot modules exist, and adjust start pointer past all of them */
    if ((mbi_virtual->flags & (1U << 3)) && mbi_virtual->mods_count > 0) {
        uint32_t highest_mod_end = 0;
        
        /* Create a separate virtual pointer for dereferencing the module */
        uint32_t current_mod_addr_virt = mbi_virtual->mods_addr;
        if (current_mod_addr_virt < KERNEL_VIRTUAL_OFFSET)
            current_mod_addr_virt += KERNEL_VIRTUAL_OFFSET;

        for (uint32_t i = 0; i < mbi_virtual->mods_count; i++) {
            uint32_t mod_end = *(uint32_t *)(current_mod_addr_virt + 4);
            
            if (mod_end > highest_mod_end)
                highest_mod_end = mod_end;

            current_mod_addr_virt += 16;
        }

        /* Page align the highest module end address */
        auto aligned_mod_end = (highest_mod_end + 0x0000'0FFFU) & ~0x0000'0FFFU;

        /* If the modules end after the kernel, move the allocation start past them */
        if (aligned_mod_end > physical_alloc_start)
            physical_alloc_start = aligned_mod_end;
    }

    /* Determine upper memory bounds */
    uint64_t mem_high_point = 0;
    if (mbi_virtual->flags & (1U << 0))
        mem_high_point = (uint64_t)mbi_virtual->mem_upper * 1024U;

    if (mem_high_point > MAX_PHYSICAL_BYTES || mem_high_point == 0)
        mem_high_point = MAX_PHYSICAL_BYTES;

    qemu_printf(QEMU_MEM, QEMU_INFO, "Memory map: (physical allocation start: %p, memory high point: %p)",
                physical_alloc_start, (uint32_t)mem_high_point);

    /* Initialize VMM & PMM */
    auto post_paging_free_mem_phys = vmm_initialize((uint32_t)mem_high_point, physical_alloc_start);

    /* Initialize Heap */
    constexpr uint32_t heap_size = 4'194'304; /* 4MB initial size */
    auto post_paging_free_mem_virt = post_paging_free_mem_phys + (uint32_t)&_kernel_start;

    heap_initialize((void *)post_paging_free_mem_virt, heap_size);
}
