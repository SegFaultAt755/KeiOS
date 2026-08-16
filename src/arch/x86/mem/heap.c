/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "arch/x86/heap.h"
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
    auto aligned_addr = HEAP_ALIGN_UP((uint32_t)start_addr, HEAP_ALIGNMENT);
    auto lost_bytes = aligned_addr - (uint32_t)start_addr;

    if (total_size <= (sizeof(struct heap_segment) + lost_bytes)) {
        qemu_printf(QEMU_MEM, QEMU_ERROR,
                    "Failed to initialize heap (total requested size is less than minimum required)");
        return;
    }

    heap_start = (struct heap_segment *)aligned_addr;
    auto usable_len = total_size - lost_bytes - sizeof(struct heap_segment);
    heap_set_len_and_flags(heap_start, usable_len, true);
    heap_start->next = nullptr;
    heap_start->prev = nullptr;

    qemu_printf(QEMU_MEM, QEMU_OK, "Heap initialized (start point: %p, length: %d)", (uint32_t)heap_start, usable_len);
}

static void split_segment(struct heap_segment *segment, uint32_t requested_size) {
    auto current_len = heap_get_len(segment);
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
    if (size == 0 || heap_start == nullptr)
        return nullptr;

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

    return nullptr;
}

void kfree(void *ptr) {
    if (ptr == nullptr)
        return;

    struct heap_segment *segment = (struct heap_segment *)((uint32_t)ptr - sizeof(struct heap_segment));
    heap_set_len_and_flags(segment, heap_get_len(segment), true);

    /* Merge forward */
    if (segment->next != nullptr && heap_is_free(segment->next)) {
        auto merged_len = heap_get_len(segment) + sizeof(struct heap_segment) + heap_get_len(segment->next);
        heap_set_len_and_flags(segment, merged_len, true);
        segment->next = segment->next->next;
        if (segment->next != nullptr)
            segment->next->prev = segment;
    }

    /* Merge backward */
    if (segment->prev != nullptr && heap_is_free(segment->prev)) {
        auto merged_len = heap_get_len(segment->prev) + sizeof(struct heap_segment) + heap_get_len(segment);
        heap_set_len_and_flags(segment->prev, merged_len, true);
        segment->prev->next = segment->next;
        if (segment->next != nullptr)
            segment->next->prev = segment->prev;
    }
}
