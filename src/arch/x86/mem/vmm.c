/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "arch/x86/vmm.h"
#include "arch/x86/mem.h"
#include "arch/x86/pmm.h"
#include "kernel/qemu.h"
#include "libkern/memory.h"

extern uint32_t initial_page_dir[];

static inline void invalidate(uint32_t addr) {
    __asm__ volatile("invlpg (%0)" ::"r"(addr) : "memory");
}

static inline uint32_t *get_pde(uint32_t virt_addr) {
    return (uint32_t *)(0xFFFFF000U + ((virt_addr >> 20) & 0xFFCU));
}

static inline uint32_t *get_pte(uint32_t virt_addr) {
    return (uint32_t *)(0xFFC00000U + ((virt_addr >> 10) & 0x3FFFFCU));
}

uint32_t vmm_initialize(uint32_t mem_high_point, uint32_t physical_alloc_start) {
    uint32_t *dir = (uint32_t *)initial_page_dir;
    dir[0] = 0;
    invalidate(0);

    /* Recursive page directory mapping */
    uint32_t physical_dir_addr = (uint32_t)dir - KERNEL_START;
    dir[1023] = (physical_dir_addr & PDE_FRAME) | PDE_PRESENT | PDE_RW;
    invalidate(KERNEL_START);

    /* Reload cr3 to flush tlb completely and apply recursive mapping changes */
    __asm__ volatile("mov %%cr3, %%eax\n\t"
                     "mov %%eax, %%cr3"
                     :
                     :
                     : "eax", "memory");

    /* Initialize physical frame allocation */
    pmm_initialize(mem_high_point, physical_alloc_start);

    qemu_printf(QEMU_MEM, QEMU_OK, "VMM paging enabled (page directory: 0x%x)", physical_dir_addr);
    return physical_alloc_start;
}

bool vmm_map_page(uint32_t virt_addr, uint32_t phys_addr, uint32_t flags) {
    qemu_printf(QEMU_MEM, QEMU_INFO, "Getting PDE");
    uint32_t *pde = get_pde(virt_addr);

    /* Check if the page table exists, if not, allocate a physical frame for it */
    qemu_printf(QEMU_MEM, QEMU_INFO, "0: 0x%x", pde);
    qemu_printf(QEMU_MEM, QEMU_INFO, "0: %d", *pde);
    if (!(*pde & PDE_PRESENT)) {
        qemu_printf(QEMU_MEM, QEMU_INFO, "1");
        uint32_t new_table_phys = pmm_alloc_frame();
        if (new_table_phys == 0)
            return false;
        qemu_printf(QEMU_MEM, QEMU_INFO, "2");

        *pde = (new_table_phys & PDE_FRAME) | PDE_PRESENT | PDE_RW | (flags & PDE_USER);

        uint32_t *table_base = (uint32_t *)((uint32_t)get_pte(virt_addr) & ~0xFFFU);
        qemu_printf(QEMU_MEM, QEMU_INFO, "1");
        invalidate((uint32_t)table_base);
        qemu_printf(QEMU_MEM, QEMU_INFO, "2");
        memset(table_base, 0, PAGE_SIZE);
        qemu_printf(QEMU_MEM, QEMU_INFO, "3");
    }

    qemu_printf(QEMU_MEM, QEMU_INFO, "5");

    uint32_t *pte = get_pte(virt_addr);
    *pte = (phys_addr & PTE_FRAME) | (flags & 0xFFF) | PTE_PRESENT;
    invalidate(virt_addr);
    return true;
}

void vmm_unmap_page(uint32_t virt_addr) {
    uint32_t *pde = get_pde(virt_addr);
    if (!(*pde & PDE_PRESENT))
        return;

    uint32_t *pte = get_pte(virt_addr);
    *pte = 0;
    invalidate(virt_addr);
}

uint32_t vmm_get_phys(uint32_t virt_addr) {
    uint32_t *pde = get_pde(virt_addr);
    if (!(*pde & PDE_PRESENT))
        return 0;

    uint32_t *pte = get_pte(virt_addr);
    if (!(*pte & PTE_PRESENT))
        return 0;

    return (*pte & PTE_FRAME) + (virt_addr & 0xFFF);
}
