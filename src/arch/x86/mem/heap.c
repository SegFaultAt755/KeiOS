/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "arch/x86/heap.h"
#include "kernel/panic.h"
#include "kernel/qemu.h"

static struct heap_segment *heap_start = nullptr;

[[nodiscard]] static inline bool heap_is_free(struct heap_segment *seg) {
    return (seg->len_flags & HEAP_FLAG_FREE) != 0;
}

[[nodiscard]] static inline uint32_t heap_get_len(struct heap_segment *seg) {
    return seg->len_flags & HEAP_LEN_MASK;
}

static inline void heap_set_len_and_flags(struct heap_segment *seg, uint32_t len, bool is_free) {
    seg->len_flags = (len & HEAP_LEN_MASK) | (is_free ? HEAP_FLAG_FREE : 0);
}

void heap_init(void *start_addr, uint32_t total_size) {
    KERNEL_ASSERT(start_addr != nullptr, "Heap initialization error", "Heap start address cannot be NULL");

    KERNEL_ASSERT(total_size > 0, "Heap initialization error", "Heap total size must be greater than zero");

    auto aligned_addr = HEAP_ALIGN_UP((uint32_t)start_addr, HEAP_ALIGNMENT);
    auto lost_bytes = aligned_addr - (uint32_t)start_addr;

    KERNEL_ASSERT(total_size > (sizeof(struct heap_segment) + lost_bytes), "Heap initialization error",
                  "Total requested size is less than minimum required for heap header and alignment");

    heap_start = (struct heap_segment *)aligned_addr;
    auto usable_len = total_size - lost_bytes - sizeof(struct heap_segment);
    heap_set_len_and_flags(heap_start, usable_len, true);
    heap_start->next = nullptr;
    heap_start->prev = nullptr;

    qemu_printf(QEMU_MEM, QEMU_OK, "Heap initialized (start point: %p, length: %d)", (uint32_t)heap_start, usable_len);
}

static void split_segment(struct heap_segment *segment, uint32_t requested_size) {
    KERNEL_ASSERT(segment != nullptr, "Heap corruption detected", "Cannot split a NULL heap segment");

    auto current_len = heap_get_len(segment);

    KERNEL_ASSERT(current_len >= requested_size, "Heap corruption detected",
                  "Segment length is less than requested allocation size");

    auto remaining_memory = current_len - requested_size;

    if (remaining_memory >= (sizeof(struct heap_segment) + HEAP_MIN_SPLIT_SIZE)) {
        auto new_header_addr = (uint32_t)segment + sizeof(struct heap_segment) + requested_size;
        struct heap_segment *new_segment = (struct heap_segment *)new_header_addr;

        heap_set_len_and_flags(new_segment, remaining_memory - sizeof(struct heap_segment), true);
        new_segment->next = segment->next;
        new_segment->prev = segment;

        if (segment->next != nullptr)
            segment->next->prev = new_segment;

        segment->next = new_segment;
        heap_set_len_and_flags(segment, requested_size, heap_is_free(segment));
    }
}

[[nodiscard]] void *kmalloc(uint32_t size) {
    KERNEL_ASSERT(heap_start != nullptr, "Heap allocator not initialized",
                  "kmalloc called before heap_init - heap manager is not ready");

    KERNEL_ASSERT(size > 0, "Invalid heap allocation request",
                  "kmalloc called with size = 0 - allocation size must be greater than zero");

    auto aligned_size = HEAP_ALIGN_UP(size, HEAP_ALIGNMENT);
    struct heap_segment *current = heap_start;

    while (current != nullptr) {
        if (heap_is_free(current) && heap_get_len(current) >= aligned_size) {
            split_segment(current, aligned_size);
            heap_set_len_and_flags(current, heap_get_len(current), false);
            return (void *)((uint32_t)current + sizeof(struct heap_segment));
        }
        current = current->next;
    }

    /* No free memory block found - kernel heap exhausted */
    KERNEL_PANIC_FORMAT("Kernel heap exhaustion",
                        "kmalloc failed to allocate %u bytes - no free heap segments available", size);
}

void kfree(void *ptr) {
    if (ptr == nullptr)
        return;

    KERNEL_ASSERT(heap_start != nullptr, "Heap allocator not initialized",
                  "kfree called before heap_init - heap manager is not ready");

    struct heap_segment *segment = (struct heap_segment *)((uint32_t)ptr - sizeof(struct heap_segment));

    /* Basic sanity check: verify pointer is within reasonable bounds */
    KERNEL_ASSERT((uint32_t)segment >= (uint32_t)heap_start, "Invalid pointer to kfree",
                  "kfree called with pointer before heap start - memory corruption or invalid pointer");

    KERNEL_ASSERT(!heap_is_free(segment), "Double-free detected",
                  "kfree called on already freed memory block - double free or heap corruption");

    heap_set_len_and_flags(segment, heap_get_len(segment), true);

    /* Merge with the next free block */
    if (segment->next != nullptr && heap_is_free(segment->next)) {
        KERNEL_ASSERT(segment->next != segment, "Heap corruption detected",
                      "Segment next pointer equals itself - circular link detected");

        auto merged_len = heap_get_len(segment) + sizeof(struct heap_segment) + heap_get_len(segment->next);
        heap_set_len_and_flags(segment, merged_len, true);
        segment->next = segment->next->next;
        if (segment->next != nullptr)
            segment->next->prev = segment;
    }

    /* Merge with the previous free block */
    if (segment->prev != nullptr && heap_is_free(segment->prev)) {
        KERNEL_ASSERT(segment->prev != segment, "Heap corruption detected",
                      "Segment prev pointer equals itself - circular link detected");

        auto merged_len = heap_get_len(segment->prev) + sizeof(struct heap_segment) + heap_get_len(segment);
        heap_set_len_and_flags(segment->prev, merged_len, true);
        segment->prev->next = segment->next;
        if (segment->next != nullptr)
            segment->next->prev = segment->prev;
    }
}
