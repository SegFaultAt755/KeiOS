/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "arch/x86/pmm.h"
#include "arch/x86/mem.h"
#include "kernel/qemu.h"
#include "libkern/memory.h"

#define TOTAL_PAGES (MAX_PHYSICAL_BYTES / PAGE_SIZE)

static uint8_t physical_mem_bitmap[TOTAL_PAGES / 8];
static uint32_t total_frames = 0;

void pmm_initialize(uint64_t mem_high_point, uint32_t physical_alloc_start) {
    uint32_t page_frame_min = ((physical_alloc_start + PAGE_SIZE) - 1) / PAGE_SIZE;
    total_frames = (mem_high_point > MAX_PHYSICAL_BYTES ? MAX_PHYSICAL_BYTES : mem_high_point) / PAGE_SIZE;

    /* Mark everything as free initially */
    memset(physical_mem_bitmap, 0, sizeof(physical_mem_bitmap));

    /* Mark unusable high memory past detected ram as allocated */
    for (uint32_t i = total_frames; i < TOTAL_PAGES; i++)
        physical_mem_bitmap[i / 8] |= (1U << (i % 8));

    /* Protect kernel, modules, lower memory structures from being overwritten */
    for (uint32_t i = 0; i < page_frame_min; i++)
        physical_mem_bitmap[i / 8] |= (1U << (i % 8));

    qemu_printf(QEMU_MEM, QEMU_OK, "PMM initialized (total frames: %d)", total_frames);
}

uint32_t pmm_alloc_frame(void) {
    for (uint32_t i = 0; i < total_frames; i++) {
        if ((physical_mem_bitmap[i / 8] & (1U << (i % 8))) == 0) {
            physical_mem_bitmap[i / 8] |= (1U << (i % 8));
            return i * PAGE_SIZE;
        }
    }

    return 0; /* Out of memory */
}

void pmm_free_frame(uint32_t frame_addr) {
    uint32_t frame = frame_addr / PAGE_SIZE;
    if (frame < total_frames)
        physical_mem_bitmap[frame / 8] &= ~(1U << (frame % 8));
}
