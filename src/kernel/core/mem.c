/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "kernel/core/mem.h"
#include "arch/x86/heap.h"
#include "arch/x86/mem.h"
#include "arch/x86/pmm.h"
#include "arch/x86/vmm.h"
#include "kernel/multiboot.h"
#include "kernel/panic.h"
#include "kernel/qemu.h"

extern uint32_t _kernel_end;

void mem_init(struct multiboot_info *mbi_virtual) {
    KERNEL_ASSERT(mbi_virtual != nullptr, "Multiboot information structure missing",
                  "mem_init called with NULL multiboot info - GRUB2 bootloader did not provide memory information");

    const uint32_t virt_alloc_start = KERNEL_PAGE_ALIGN((uint32_t)&_kernel_end);
    const uint32_t phys_alloc_start = virt_alloc_start - KERNEL_VIRTUAL_OFFSET;

    if (mbi_virtual->flags & MULTIBOOT_INFO_MEMORY) {
        uint64_t mem_upper_limit = 0x0010'0000ULL + (uint64_t)mbi_virtual->mem_upper * 1024U;
        if (mem_upper_limit > MAX_PHYSICAL_BYTES) {
            mem_upper_limit = MAX_PHYSICAL_BYTES;
        }
        qemu_printf(QEMU_MEM, QEMU_INFO, "Legacy mem_upper physical limit: %u KiB above 1 MiB (limit=%p)",
                    mbi_virtual->mem_upper, (uint32_t)mem_upper_limit);
    }

    vmm_init(mbi_virtual, phys_alloc_start);

    constexpr uint32_t heap_size = 4'194'304U; /* Initial size: 4 MB */
    const uint32_t heap_phys = pmm_alloc_contiguous(heap_size, PAGE_SIZE);
    KERNEL_ASSERT(heap_phys != 0, "Kernel heap allocation failed",
                  "Could not reserve a contiguous physical range for the kernel heap");

    for (uint32_t offset = 0; offset < heap_size; offset += PAGE_SIZE) {
        KERNEL_ASSERT(vmm_map_page(KERNEL_HEAP_VIRTUAL_START + offset, heap_phys + offset, PTE_RW),
                      "Kernel heap mapping failed", "Could not map a physical page into the kernel heap");
    }

    heap_init((void *)KERNEL_HEAP_VIRTUAL_START, heap_size);
}
