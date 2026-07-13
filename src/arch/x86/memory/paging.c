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

static struct heap_segment *heap_start = nullptr;
extern uint8_t _kernel_end;

void memory_initialize(struct multiboot_info *mbi) {
    qemu_printf(QEMU_INFO, "Initializing memory...");

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

    /* Initialize the Heap */
    uint32_t heap_size = 1024 * 1024 * 4;
    heap_initialize((void *)post_paging_free_mem, heap_size);

    qemu_printf(QEMU_INFO, "Kernel Heap successfully initialized at 0x%x", post_paging_free_mem);
}

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

void heap_initialize(void *start_addr, uint32_t total_size) {
    uint32_t aligned_addr = HEAP_ALIGN_UP((uint32_t)start_addr, HEAP_ALIGNMENT);
    uint32_t lost_bytes = aligned_addr - (uint32_t)start_addr;

    if (total_size <= (sizeof(struct heap_segment) + lost_bytes))
        return;

    /* Set up the first free memory block covering the rest of the available space */
    heap_start = (struct heap_segment *)aligned_addr;
    heap_start->len = total_size - lost_bytes - sizeof(struct heap_segment);
    heap_start->next = nullptr;
    heap_start->prev = nullptr;
    heap_start->is_free = true;
}

static void split_segment(struct heap_segment *segment, uint32_t requested_size) {
    uint32_t remaining_memory = segment->len - requested_size;

    /* Split the block only if the leftover space can hold a new header and minimum data payload */
    if (remaining_memory >= (sizeof(struct heap_segment) + HEAP_MIN_SPLIT_SIZE)) {
        uint32_t new_header_addr = (uint32_t)segment + sizeof(struct heap_segment) + requested_size;
        struct heap_segment *new_segment = (struct heap_segment *)new_header_addr;

        new_segment->len = remaining_memory - sizeof(struct heap_segment);
        new_segment->is_free = true;
        new_segment->next = segment->next;
        new_segment->prev = segment;

        if (segment->next != nullptr)
            segment->next->prev = new_segment;

        segment->next = new_segment;
        segment->len = requested_size;
    }
}

void *kmalloc(uint32_t size) {
    /* Return early if requesting 0 bytes or if the heap isn't initialized */
    if (size == 0 || heap_start == nullptr)
        return nullptr;

    uint32_t aligned_size = HEAP_ALIGN_UP(size, HEAP_ALIGNMENT);
    struct heap_segment *current = heap_start;

    /* Search the linked list of blocks for a free space that fits the requested size */
    while (current != nullptr) {
        if (current->is_free && current->len >= aligned_size) {
            split_segment(current, aligned_size);
            current->is_free = false;
            return (void *)((uint32_t)current + sizeof(struct heap_segment));
        }
        current = current->next;
    }

    return nullptr;
}

void kfree(void *ptr) {
    if (ptr == nullptr)
        return;

    /* Calculate the header's start address by stepping back from the user data pointer */
    struct heap_segment *segment = (struct heap_segment *)((uint32_t)ptr - sizeof(struct heap_segment));
    segment->is_free = true;

    /* If the next block in memory is free, merge it into the current block */
    if (segment->next != nullptr && segment->next->is_free) {
        segment->len += sizeof(struct heap_segment) + segment->next->len;
        segment->next = segment->next->next;
        if (segment->next != nullptr)
            segment->next->prev = segment;
    }

    /* If the previous block in memory is also free, merge the current block into it */
    if (segment->prev != nullptr && segment->prev->is_free) {
        segment->prev->len += sizeof(struct heap_segment) + segment->len;
        segment->prev->next = segment->next;
        if (segment->next != nullptr)
            segment->next->prev = segment->prev;
    }
}
