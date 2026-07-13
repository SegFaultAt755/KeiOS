/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "arch/x86/mem.h"
#include "kernel/qemu.h"
#include "libkern/memory.h"

extern void *initial_page_dir;

#define TOTAL_PAGES (MAX_PHYSICAL_BYTES / PAGE_SIZE)
uint8_t physical_mem_bitmap[TOTAL_PAGES / 8];

#define MAX_DIRECTORIES 256
struct page_dir page_dirs[MAX_DIRECTORIES];
uint32_t page_dirs_used[MAX_DIRECTORIES];

static inline void invalidate(uint32_t addr) {
    __asm__ volatile("invlpg %0" ::"m"(addr));
}

void paging_set_physical_memory(uint32_t mem_high_point, uint32_t physical_alloc_start) {
    uint32_t page_frame_min = ((physical_alloc_start + PAGE_SIZE) - 1) / PAGE_SIZE;
    [[maybe_unused]] uint32_t page_frame_max = mem_high_point / PAGE_SIZE;

    /* Mark everything as free initially (0 free, 1 allocated) */
    memset(physical_mem_bitmap, 0, sizeof(physical_mem_bitmap));

    /* Protect the kernel and modules */
    /* Mark the kernel code as allocated to avoid accidental allocation */
    for (uint32_t i = 0; i < page_frame_min; i++)
        physical_mem_bitmap[i / 8] |= (1 << (i % 8));
}

uint32_t paging_initialize(uint32_t mem_high_point, uint32_t physical_alloc_start) {
    qemu_printf(QEMU_INFO, "Paging memory");

    struct page_dir *dir = (struct page_dir *)initial_page_dir;
    memset(&dir->entries[0], 0, sizeof(struct page_dir_entry));
    invalidate(0);

    /* Recursive mapping from the end */
    uint32_t physical_dir_addr = (uint32_t)dir - PAGING_KERNEL_START;

    dir->entries[1023].present = 1;
    dir->entries[1023].rw = 1;
    dir->entries[1023].table_addr = physical_dir_addr >> 12;

    invalidate(PAGING_KERNEL_END);
    paging_set_physical_memory(mem_high_point, physical_alloc_start);

    memset(page_dirs, 0, sizeof(page_dirs));
    memset(page_dirs_used, 0, sizeof(page_dirs_used));

    return physical_alloc_start;
}
