/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "arch/x86/mem.h"
#include "kernel/qemu.h"

static struct heap_segment *heap_start = nullptr;

void heap_initialize(void *start_addr, uint32_t total_size) {
    qemu_printf(QEMU_INFO, "Initializing kernel heap at 0x%x", (uint32_t)start_addr);

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
