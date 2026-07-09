#include "arch/x86/paging.h"
#include "libkern/memory.h"
#include "kernel/qemu.h"

extern void *initial_page_directory;

#define TOTAL_PAGES (MAX_PHYSICAL_BYTES / PAGE_SIZE)
uint8_t physical_memory_bitmap[TOTAL_PAGES / 8]; 

#define MAX_DIRECTORIES 256
struct page_directory page_directories[MAX_DIRECTORIES];
uint32_t page_directories_used[MAX_DIRECTORIES];

static inline void invalidate(uint32_t address) {
    __asm__ volatile ("invlpg %0" :: "m" (address));
}

void paging_set_physical_memory(uint32_t memory_high_point, uint32_t physical_allocation_start) {
    qemu_printf(QEMU_LOG_INFO, "Calculating page frames");
    uint32_t page_frame_min = ((physical_allocation_start + PAGE_SIZE) - 1) / PAGE_SIZE;
    uint32_t page_frame_max = memory_high_point / PAGE_SIZE;

    /* Automatically scale to the calculated array size */
    qemu_printf(QEMU_LOG_INFO, "Scale to array with size %d", sizeof(physical_memory_bitmap));
    memset(physical_memory_bitmap, 0, sizeof(physical_memory_bitmap));
}

void paging_initialize(uint32_t memory_high_point, uint32_t physical_allocation_start) {
    qemu_printf(QEMU_LOG_INFO, "Resetting first point from initial page directories");
    
    struct page_directory initial_page_directory_compiled = *(struct page_directory *) initial_page_directory;
    memset(&initial_page_directory_compiled.entries[0], 0, sizeof(struct page_directory_entry));
    invalidate(0);

    /* Recursive mapping from the end */
    qemu_printf(QEMU_LOG_INFO, "Recursive mapping from the end");
    uint32_t physical_directory_address = (uint32_t) &initial_page_directory_compiled - PAGING_KERNEL_START;

    initial_page_directory_compiled.entries[1023].present = 1;
    initial_page_directory_compiled.entries[1023].rw = 1;
    initial_page_directory_compiled.entries[1023].table_addr = physical_directory_address >> 12; /* Shift to store only the high 20 bits */

    qemu_printf(QEMU_LOG_INFO, "Setting physical memory");
    invalidate(PAGING_KERNEL_END);
    paging_set_physical_memory(memory_high_point, physical_allocation_start);

    qemu_printf(QEMU_LOG_INFO, "Resetting page directories");
    memset(page_directories, 0, sizeof(page_directories));
    memset(page_directories_used, 0, sizeof(page_directories_used));
}
