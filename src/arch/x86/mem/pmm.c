/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "arch/x86/pmm.h"
#include "arch/x86/mem.h"
#include "kernel/panic.h"
#include "kernel/qemu.h"
#include "libkern/memory.h"

constexpr size_t TOTAL_PAGES = MAX_PHYSICAL_BYTES / PAGE_SIZE;

static uint8_t physical_mem_bitmap[TOTAL_PAGES / 8];
static uint32_t total_frames;

static const uint8_t *boot_phys_to_virt(uint32_t phys_addr, uint32_t size) {
    uint64_t end = (uint64_t)phys_addr + size;
    if (phys_addr == 0 || end > MULTIBOOT_BOOTSTRAP_MAPPED_BYTES) {
        return nullptr;
    }

    return (const uint8_t *)ADD_KERNEL_OFFSET(phys_addr);
}

static void set_frames(uint64_t start, uint64_t end, bool allocated) {
    if (start >= end) {
        return;
    }

    if (end > MAX_PHYSICAL_BYTES) {
        end = MAX_PHYSICAL_BYTES;
    }
    if (start >= end || start >= MAX_PHYSICAL_BYTES) {
        return;
    }

    uint64_t first = allocated ? start / PAGE_SIZE : (start + PAGE_MASK) / PAGE_SIZE;
    uint64_t last = allocated ? (end + PAGE_MASK) / PAGE_SIZE : end / PAGE_SIZE;
    if (first < 1) {
        first = 1;
    }
    if (last > total_frames) {
        last = total_frames;
    }

    for (uint64_t frame = first; frame < last; frame++) {
        uint8_t mask = (uint8_t)(1U << (frame % 8));
        if (allocated) {
            physical_mem_bitmap[frame / 8] |= mask;
        } else {
            physical_mem_bitmap[frame / 8] &= (uint8_t)~mask;
        }
    }
}

static uint64_t read_memory_map(const struct multiboot_info *mbi, uint8_t pass) {
    if (!(mbi->flags & MULTIBOOT_INFO_MEM_MAP) || mbi->mmap_len == 0) {
        KERNEL_PANIC("Multiboot memory map missing", "The physical allocator requires a Multiboot memory map");
    }

    const uint8_t *map = boot_phys_to_virt(mbi->mmap_addr, mbi->mmap_len);
    if (!map) {
        KERNEL_PANIC("Invalid Multiboot memory map", "Memory map is outside the bootstrap-mapped physical range");
    }

    uint64_t highest_addr = 0;
    size_t offset = 0;
    while (offset < mbi->mmap_len) {
        size_t remaining = mbi->mmap_len - offset;
        if (remaining < sizeof(uint32_t)) {
            KERNEL_PANIC("Malformed Multiboot memory map", "Truncated memory-map entry size");
        }

        uint32_t entry_size;
        memcpy(&entry_size, map + offset, sizeof(entry_size));
        if (entry_size < sizeof(struct multiboot_mmap_entry) - sizeof(uint32_t) ||
            entry_size > remaining - sizeof(uint32_t)) {
            KERNEL_PANIC("Malformed Multiboot memory map", "Invalid memory-map entry length");
        }

        const struct multiboot_mmap_entry *entry = (const struct multiboot_mmap_entry *)(map + offset);
        uint64_t base = ((uint64_t)entry->addr_upper << 32) | entry->addr_lower;
        uint64_t length = ((uint64_t)entry->len_upper << 32) | entry->len_lower;
        if (length == 0) {
            offset += sizeof(uint32_t) + entry_size;
            continue;
        }
        uint64_t end = base + length;
        if (end < base) {
            KERNEL_PANIC("Malformed Multiboot memory map", "Memory-map entry range overflows");
        }

        if (end > highest_addr) {
            highest_addr = end;
        }
        if (pass == 1 && entry->type == MULTIBOOT_MEMORY_AVAILABLE) {
            set_frames(base, end, false);
        } else if (pass == 2 && entry->type != MULTIBOOT_MEMORY_AVAILABLE) {
            set_frames(base, end, true);
        }

        offset += sizeof(uint32_t) + entry_size;
    }

    return highest_addr;
}

static void reserve_boot_structures(const struct multiboot_info *mbi) {
    uintptr_t mbi_addr = (uintptr_t)mbi;
    if (mbi_addr >= KERNEL_VIRTUAL_OFFSET) {
        mbi_addr -= KERNEL_VIRTUAL_OFFSET;
    }
    pmm_reserve_phys(mbi_addr, sizeof(*mbi));
    pmm_reserve_phys(mbi->mmap_addr, mbi->mmap_len);

    if (mbi->cmd_line) {
        pmm_reserve_phys(mbi->cmd_line, 4096);
    }
    if (mbi->boot_loader_name) {
        pmm_reserve_phys(mbi->boot_loader_name, 4096);
    }

    if (mbi->flags & MULTIBOOT_INFO_MODS) {
        if (mbi->mods_count > 256) {
            KERNEL_PANIC("Too many Multiboot modules", "Module count exceeds the supported maximum of 256");
        }
        if (mbi->mods_count > 0) {
            uint64_t modules_size = (uint64_t)mbi->mods_count * sizeof(struct multiboot_module);
            const uint8_t *module_list = boot_phys_to_virt(mbi->mods_addr, (uint32_t)modules_size);
            if (!module_list) {
                KERNEL_PANIC("Invalid Multiboot modules", "Module descriptors are outside the bootstrap-mapped range");
            }

            pmm_reserve_phys(mbi->mods_addr, modules_size);
            const struct multiboot_module *modules = (const struct multiboot_module *)module_list;
            for (uint32_t i = 0; i < mbi->mods_count; i++) {
                if (modules[i].mod_end < modules[i].mod_start) {
                    KERNEL_PANIC("Malformed Multiboot module", "Module end address precedes its start address");
                }
                pmm_reserve_phys(modules[i].mod_start, modules[i].mod_end - modules[i].mod_start);
                if (modules[i].cmdline) {
                    pmm_reserve_phys(modules[i].cmdline, 4096);
                }
            }
        }
    }

    if ((mbi->flags & MULTIBOOT_INFO_FRAMEBUFFER_INFO) && mbi->framebuffer_addr < MAX_PHYSICAL_BYTES) {
        uint64_t framebuffer_size = (uint64_t)mbi->framebuffer_pitch * mbi->framebuffer_height;
        pmm_reserve_phys(mbi->framebuffer_addr, framebuffer_size);
    }
}

void pmm_init(const struct multiboot_info *mbi, uint32_t physical_alloc_start) {
    if (!mbi) {
        KERNEL_PANIC("Multiboot information missing", "pmm_init requires the boot information structure");
    }

    memset(physical_mem_bitmap, 0xFF, sizeof(physical_mem_bitmap));
    uint64_t highest_addr = read_memory_map(mbi, 0);
    if (highest_addr > MAX_PHYSICAL_BYTES) {
        highest_addr = MAX_PHYSICAL_BYTES;
    }
    total_frames = (uint32_t)(highest_addr / PAGE_SIZE);
    if (total_frames == 0) {
        KERNEL_PANIC("No physical memory available", "Multiboot memory map contains no addressable RAM");
    }

    read_memory_map(mbi, 1);
    read_memory_map(mbi, 2);
    set_frames(0, physical_alloc_start, true);
    reserve_boot_structures(mbi);

    qemu_printf(QEMU_MEM, QEMU_OK, "PMM initialized (addressable frame slots: %u)", total_frames);
}

[[nodiscard]] uint32_t pmm_alloc_frame() {
    for (uint32_t frame = 1; frame < total_frames; frame++) {
        if ((physical_mem_bitmap[frame / 8] & (1U << (frame % 8))) == 0) {
            physical_mem_bitmap[frame / 8] |= (uint8_t)(1U << (frame % 8));
            return frame * PAGE_SIZE;
        }
    }

    KERNEL_PANIC("Physical memory exhaustion", "pmm_alloc_frame failed - no free physical page frames available");
}

uint32_t pmm_alloc_contiguous(uint32_t size, uint32_t alignment) {
    if (size == 0 || size > UINT32_MAX - PAGE_MASK || alignment < PAGE_SIZE || (alignment & PAGE_MASK) != 0) {
        return 0;
    }

    uint32_t pages = (size + PAGE_MASK) / PAGE_SIZE;
    if (pages > total_frames) {
        return 0;
    }
    uint32_t alignment_frames = alignment / PAGE_SIZE;
    for (uint32_t first = 1; first <= total_frames - pages;) {
        uint32_t remainder = first % alignment_frames;
        if (remainder != 0) {
            first += alignment_frames - remainder;
            continue;
        }

        uint32_t page = 0;
        while (page < pages && (physical_mem_bitmap[(first + page) / 8] & (1U << ((first + page) % 8))) == 0) {
            page++;
        }
        if (page == pages) {
            for (page = 0; page < pages; page++) {
                physical_mem_bitmap[(first + page) / 8] |= (uint8_t)(1U << ((first + page) % 8));
            }
            return first * PAGE_SIZE;
        }
        first += page + 1;
    }

    return 0;
}

void pmm_reserve_phys(uint64_t phys_addr, uint64_t size) {
    if (size == 0 || phys_addr >= MAX_PHYSICAL_BYTES) {
        return;
    }

    uint64_t end = phys_addr + size;
    if (end < phys_addr || end > MAX_PHYSICAL_BYTES) {
        end = MAX_PHYSICAL_BYTES;
    }
    set_frames(phys_addr, end, true);
}
