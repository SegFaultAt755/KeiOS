/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "kernel/acpi.h"
#include "kernel/panic.h"
#include "kernel/rsdp.h"
#include "libkern/memory.h"

#if defined(__i386__) || defined(_M_IX86)
#include "arch/x86/mem.h"
#include "arch/x86/pmm.h"
#include "arch/x86/vmm.h"
#else
#error "Unsupported architecture! (i386 is available)"
#endif

struct fadt *global_fadt = nullptr;
struct dsdt *global_dsdt = nullptr;

constexpr uint32_t ACPI_MAPPING_BASE = 0xD000'0000U;
constexpr uint32_t ACPI_MAPPING_LIMIT = 0xE000'0000U;
static uint32_t acpi_mapping_next = ACPI_MAPPING_BASE;

void *acpi_map_range(uint32_t phys_addr, uint32_t length) {
    if (phys_addr == 0 || length == 0) {
        return nullptr;
    }

    uint64_t physical_end = (uint64_t)phys_addr + length;
    if (physical_end > MAX_PHYSICAL_BYTES) {
        return nullptr;
    }

    uint32_t page_offset = phys_addr & PAGE_MASK;
    uint64_t mapping_size = ((uint64_t)page_offset + length + PAGE_MASK) & ~(uint64_t)PAGE_MASK;
    uint64_t virtual_end = (uint64_t)acpi_mapping_next + mapping_size;
    if (virtual_end > ACPI_MAPPING_LIMIT) {
        return nullptr;
    }

    uint32_t physical_page = phys_addr & ~PAGE_MASK;
    uint32_t mapping_start = acpi_mapping_next;
    for (uint64_t offset = 0; offset < mapping_size; offset += PAGE_SIZE) {
        uint32_t page = physical_page + (uint32_t)offset;
        pmm_reserve_phys(page, PAGE_SIZE);
        if (!vmm_map_page(mapping_start + (uint32_t)offset, page, PTE_RW | PDE_PWT)) {
            return nullptr;
        }
    }

    acpi_mapping_next = (uint32_t)virtual_end;
    return (void *)(mapping_start + page_offset);
}

/**
 * Initialize ACPI by locating and mapping the root table (RSDT or XSDT).
 * @param rsdp Pointer to the RSDP (Root System Description Pointer)
 * @return Pointer to the FADT
 */
[[nodiscard]]
struct fadt *acpi_init(struct rsdp *rsdp) {
    if (!rsdp) {
        KERNEL_PANIC("Null RSDP pointer", "ACPI initialization failed");
    }

    /* Determine whether ACPI uses an XSDT or an RSDT */
    bool is_xsdt = (rsdp->revision >= 2);
    uint64_t root_phys_addr = 0;

    if (is_xsdt) {
        struct rsdp20 *rsdp20 = (struct rsdp20 *)rsdp;
        root_phys_addr = rsdp20->xsdt_address;
    } else {
        root_phys_addr = rsdp->rsdt_address;
    }

    if (root_phys_addr == 0) {
        KERNEL_PANIC("Missing ACPI root table", "RSDP root table address is null");
    }
    if (root_phys_addr > UINT32_MAX) {
        KERNEL_PANIC("ACPI root table address is unsupported",
                     "The XSDT address exceeds the i386 physical address space");
    }

    struct acpi_header *root_table = acpi_map_range((uint32_t)root_phys_addr, sizeof(struct acpi_header));
    if (!root_table) {
        KERNEL_PANIC("Could not map ACPI root-table header", "ACPI initialization failed");
    }

    if (root_table->length < sizeof(struct acpi_header)) {
        KERNEL_PANIC("ACPI root table length is invalid", "RSDP root table is corrupted");
    }

    root_table = acpi_map_range((uint32_t)root_phys_addr, root_table->length);
    if (!root_table) {
        KERNEL_PANIC("Could not map complete ACPI root table", "ACPI initialization failed");
    }

    struct acpi_header *fadt_header = find_fadt(root_table, is_xsdt);
    if (!fadt_header)
        KERNEL_PANIC("Failed to find FACP signature in root table", "Invalid root table from RSDP");

    if (fadt_header->length < sizeof(struct fadt)) {
        KERNEL_PANIC("FADT length is invalid", "ACPI FADT does not contain the fields required by KeiOS");
    }

    return (struct fadt *)fadt_header;
}
