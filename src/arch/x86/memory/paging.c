#include "arch/x86/paging.h"
#include "libkern/memory.h"
#include "kernel/qemu.h"

extern void *initial_page_dir;

#define TOTAL_PAGES (MAX_PHYSICAL_BYTES / PAGE_SIZE)
uint8_t physical_mem_bitmap[TOTAL_PAGES / 8]; 

#define MAX_DIRECTORIES 256
struct page_dir page_dirs[MAX_DIRECTORIES];
uint32_t page_dirs_used[MAX_DIRECTORIES];

static inline void invalidate(uint32_t addr) {
    __asm__ volatile ("invlpg %0" :: "m" (addr));
}

void paging_set_physical_memory(uint32_t mem_high_point, uint32_t physical_alloc_start) {
    qemu_printf(QEMU_LOG_INFO, "Calculating page frames");
    uint32_t page_frame_min = ((physical_alloc_start + PAGE_SIZE) - 1) / PAGE_SIZE;
    uint32_t page_frame_max = mem_high_point / PAGE_SIZE;

    /* Automatically scale to the calculated array size */
    qemu_printf(QEMU_LOG_INFO, "Scale to array with size %d", sizeof(physical_mem_bitmap));
    memset(physical_mem_bitmap, 0, sizeof(physical_mem_bitmap));
}

void paging_initialize(uint32_t mem_high_point, uint32_t physical_alloc_start) {
    qemu_printf(QEMU_LOG_INFO, "Resetting first point from initial page directories");
    
    struct page_dir initial_page_dir_c = *(struct page_dir *) initial_page_dir;
    memset(&initial_page_dir_c.entries[0], 0, sizeof(struct page_dir_entry));
    invalidate(0);

    /* Recursive mapping from the end */
    qemu_printf(QEMU_LOG_INFO, "Recursive mapping from the end");
    uint32_t physical_dir_addr = (uint32_t) &initial_page_dir_c - PAGING_KERNEL_START;

    initial_page_dir_c.entries[1023].present = 1;
    initial_page_dir_c.entries[1023].rw = 1;
    initial_page_dir_c.entries[1023].table_addr = physical_dir_addr >> 12; /* Shift to store only the high 20 bits */

    qemu_printf(QEMU_LOG_INFO, "Setting physical memory");
    invalidate(PAGING_KERNEL_END);
    paging_set_physical_memory(mem_high_point, physical_alloc_start);

    qemu_printf(QEMU_LOG_INFO, "Resetting page directories");
    memset(page_dirs, 0, sizeof(page_dirs));
    memset(page_dirs_used, 0, sizeof(page_dirs_used));
}
