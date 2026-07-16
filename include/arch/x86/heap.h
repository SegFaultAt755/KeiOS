#pragma once

/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include <stdint.h>

#define HEAP_ALIGNMENT 16
#define HEAP_ALIGN_UP(val, align) (((val) + ((align) - 1)) & ~((align) - 1))
#define HEAP_MIN_SPLIT_SIZE 32

/* Bit 0 of len_flags indicates whether the segment is free */
#define HEAP_FLAG_FREE (1U << 0)
#define HEAP_LEN_MASK  (~(uint32_t)0xFU)

struct heap_segment {
    uint32_t len_flags; /* Length upper 28 bits, flags lower 4 bits */
    struct heap_segment *next;
    struct heap_segment *prev;
};

void heap_initialize(void *start_addr, uint32_t total_size);
void *kmalloc(uint32_t size);
void kfree(void *ptr);
