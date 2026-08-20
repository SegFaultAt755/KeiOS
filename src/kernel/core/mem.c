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

    /* Get the physical address of the Multiboot information structure */
    uint32_t mbi_phys = (uint32_t)mbi_virtual;
    if (mbi_phys >= KERNEL_VIRTUAL_OFFSET)
        mbi_phys -= KERNEL_VIRTUAL_OFFSET;

    /* Reserve the physical memory used by the Multiboot information structure */
    const uint32_t mbi_end = mbi_phys + sizeof(struct multiboot_info);
    const uint32_t aligned_mbi_end = KERNEL_PAGE_ALIGN(mbi_end);

    if (aligned_mbi_end > phys_alloc_start)
        phys_alloc_start = aligned_mbi_end;

    /* If modules exist, move the allocation start past all of them */
    if ((mbi_virtual->flags & (1U << 3)) && mbi_virtual->mods_count > 0) {
        uint32_t highest_mod_end = 0;

        /* Create a virtual pointer that can be used to read the module list */
        uint32_t mod_addr_virt = mbi_virtual->mods_addr;
        if (mod_addr_virt < KERNEL_VIRTUAL_OFFSET)
            mod_addr_virt = ADD_KERNEL_OFFSET(mod_addr_virt);

        for (uint32_t i = 0; i < mbi_virtual->mods_count; i++) {
            const uint32_t mod_end = *(uint32_t *)(mod_addr_virt + 4);

            if (mod_end > highest_mod_end)
                highest_mod_end = mod_end;

            mod_addr_virt += 16;
        }

        /* Round the highest module end address up to the next page boundary */
        const uint32_t aligned_mod_end = KERNEL_PAGE_ALIGN(highest_mod_end);

        /* If the modules end after the kernel, start allocation after the modules */
        if (aligned_mod_end > phys_alloc_start)
            phys_alloc_start = aligned_mod_end;
    }

    /* Find the upper limit of usable memory */
    uint64_t mem_high_point = 0;
    if (mbi_virtual->flags & (1U << 0))
        mem_high_point = (uint64_t)mbi_virtual->mem_upper * 1024U;

    if (mem_high_point > MAX_PHYSICAL_BYTES || mem_high_point == 0)
        mem_high_point = MAX_PHYSICAL_BYTES;

    qemu_printf(QEMU_MEM, QEMU_INFO, "Memory map: (physical allocation start: %p, memory high point: %p)",
                phys_alloc_start, (uint32_t)mem_high_point);

    /* Initialize the virtual and physical memory managers */
    const uint32_t post_paging_free_mem_phys = vmm_init((uint32_t)mem_high_point, phys_alloc_start);

    /* Initialize the kernel heap */
    constexpr uint32_t heap_size = 4'194'304U; /* Initial size: 4 MB */
    const uint32_t post_paging_free_mem_virt = post_paging_free_mem_phys + (uint32_t)&_kernel_start;

    heap_init((void *)post_paging_free_mem_virt, heap_size);
}
