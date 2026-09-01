/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "arch/x86/pmm.h"
#include "arch/x86/mem.h"
#include "kernel/panic.h"
#include "kernel/qemu.h"
#include "libkern/memory.h"

constexpr size_t TOTAL_PAGES = MAX_PHYSICAL_BYTES / PAGE_SIZE;

static uint8_t physical_mem_bitmap[TOTAL_PAGES / 8];
static uint32_t total_frames = 0;

void pmm_init(uint64_t mem_high_point, uint32_t physical_alloc_start) {
    auto page_frame_min = ((physical_alloc_start + PAGE_SIZE) - 1) / PAGE_SIZE;
    total_frames = (mem_high_point > MAX_PHYSICAL_BYTES ? MAX_PHYSICAL_BYTES : mem_high_point) / PAGE_SIZE;

    /* Mark all pages as free at first */
    memset(physical_mem_bitmap, 0, sizeof(physical_mem_bitmap));

    /* Mark high memory beyond detected RAM as unavailable */
    for (auto i = total_frames; i < TOTAL_PAGES; i++)
        physical_mem_bitmap[i / 8] |= (1U << (i % 8));

    /* Protect the kernel, modules, and low-memory structures from being overwritten */
    for (auto i = 0u; i < page_frame_min; i++)
        physical_mem_bitmap[i / 8] |= (1U << (i % 8));

    qemu_printf(QEMU_MEM, QEMU_OK, "PMM initialized (total frames: %d)", total_frames);
}

[[nodiscard]] uint32_t pmm_alloc_frame() {
    for (auto i = 0u; i < total_frames; i++) {
        if ((physical_mem_bitmap[i / 8] & (1U << (i % 8))) == 0) {
            physical_mem_bitmap[i / 8] |= (1U << (i % 8));
            return i * PAGE_SIZE;
        }
    }

    /* Physical memory exhausted */
    KERNEL_PANIC("Physical memory exhaustion", "pmm_alloc_frame failed - no free physical page frames available");
}

void pmm_free_frame(uint32_t frame_addr) {
    auto frame = frame_addr / PAGE_SIZE;
    if (frame < total_frames)
        physical_mem_bitmap[frame / 8] &= ~(1U << (frame % 8));
}

void pmm_reserve_phys(uint32_t phys_addr, uint32_t size) {
    if (size == 0)
        return;

    uint32_t start_frame = phys_addr / PAGE_SIZE;
    uint32_t end_frame = (phys_addr + size - 1) / PAGE_SIZE;

    /* Silently skip frames beyond detected memory - firmware tables may exist there */
    for (uint32_t i = start_frame; i <= end_frame && i < total_frames; i++)
        physical_mem_bitmap[i / 8] |= (1U << (i % 8));
}
