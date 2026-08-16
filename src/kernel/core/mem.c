/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "kernel/core/mem.h"
#include "arch/x86/heap.h"
#include "arch/x86/mem.h"
#include "arch/x86/vmm.h"
#include "kernel/multiboot.h"
#include "kernel/qemu.h"

extern uint32_t _kernel_start;
extern uint32_t _kernel_end;

void mem_init(struct multiboot_info *mbi_virtual) {
    const uint32_t virt_alloc_start = KERNEL_PAGE_ALIGN((uint32_t)&_kernel_end);
    uint32_t phys_alloc_start = virt_alloc_start - (uint32_t)&_kernel_start;

    /* Extract the physical MBI */
    uint32_t mbi_phys = (uint32_t)mbi_virtual;
    if (mbi_phys >= KERNEL_VIRTUAL_OFFSET)
        mbi_phys -= KERNEL_VIRTUAL_OFFSET;

    /* Mark physical mbi as reserved */
    const uint32_t mbi_end = mbi_phys + sizeof(struct multiboot_info);
    const uint32_t aligned_mbi_end = KERNEL_PAGE_ALIGN(mbi_end);

    if (aligned_mbi_end > phys_alloc_start)
        phys_alloc_start = aligned_mbi_end;

    /* Check if multiboot modules exist, and adjust start pointer past all of them */
    if ((mbi_virtual->flags & (1U << 3)) && mbi_virtual->mods_count > 0) {
        uint32_t highest_mod_end = 0;

        /* Create a separate virtual pointer for dereferencing the module */
        uint32_t mod_addr_virt = mbi_virtual->mods_addr;
        if (mod_addr_virt < KERNEL_VIRTUAL_OFFSET)
            mod_addr_virt += KERNEL_VIRTUAL_OFFSET;

        for (uint32_t i = 0; i < mbi_virtual->mods_count; i++) {
            const uint32_t mod_end = *(uint32_t *)(mod_addr_virt + 4);

            if (mod_end > highest_mod_end)
                highest_mod_end = mod_end;

            mod_addr_virt += 16;
        }

        /* Page align the highest module end address */
        const uint32_t aligned_mod_end = KERNEL_PAGE_ALIGN(highest_mod_end);

        /* If the modules end after the kernel, move the allocation start past them */
        if (aligned_mod_end > phys_alloc_start)
            phys_alloc_start = aligned_mod_end;
    }

    /* Determine upper memory bounds */
    uint64_t mem_high_point = 0;
    if (mbi_virtual->flags & (1U << 0))
        mem_high_point = (uint64_t)mbi_virtual->mem_upper * 1024U;

    if (mem_high_point > MAX_PHYSICAL_BYTES || mem_high_point == 0)
        mem_high_point = MAX_PHYSICAL_BYTES;

    qemu_printf(QEMU_MEM, QEMU_INFO, "Memory map: (physical allocation start: %p, memory high point: %p)",
                phys_alloc_start, (uint32_t)mem_high_point);

    /* Initialize VMM & PMM */
    const uint32_t post_paging_free_mem_phys = vmm_init((uint32_t)mem_high_point, phys_alloc_start);

    /* Initialize Heap */
    constexpr uint32_t heap_size = 4'194'304U; /* 4MB initial size */
    const uint32_t post_paging_free_mem_virt = post_paging_free_mem_phys + (uint32_t)&_kernel_start;

    heap_init((void *)post_paging_free_mem_virt, heap_size);
}
