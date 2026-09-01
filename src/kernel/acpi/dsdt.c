/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "kernel/acpi.h"
#include "kernel/panic.h"
#include "libkern/string.h"

#if defined(__i386__) || defined(_M_IX86)
#include "arch/x86/mem.h"
#include "arch/x86/pmm.h"
#include "arch/x86/vmm.h"
#else
#error "Unsupported architecture! (i386 is available)"
#endif

/**
 * Parse and map the DSDT (Differentiated System Description Table).
 * Maps the entire DSDT into kernel virtual memory with proper page allocation.
 * @param fadt Pointer to the FADT containing the DSDT physical address
 * @return Pointer to the mapped DSDT table
 */
[[nodiscard]]
struct dsdt *parse_dsdt(struct fadt *fadt) {
    if (!fadt || fadt->dsdt == 0) {
        KERNEL_PANIC("Invalid FADT pointer or DSDT physical address", "DSDT Initialization Failed");
    }

    uintptr_t dsdt_phys_addr = (uintptr_t)fadt->dsdt;

    /* Map just the first page to safely read the header */
    vmm_map_page((uint32_t)ADD_KERNEL_OFFSET(dsdt_phys_addr), (uint32_t)dsdt_phys_addr, PTE_PRESENT | PTE_RW | PDE_PWT);

    struct acpi_header *temp_header = (struct acpi_header *)ADD_KERNEL_OFFSET(dsdt_phys_addr);

    /* Calculate how many 4KB pages the entire DSDT requires */
    uint32_t page_count = (temp_header->length + 0xFFF) / 0x1000;

    /* Reserve and map all required pages for the full AML bytecode */
    for (uint32_t i = 0; i < page_count; i++) {
        uint32_t phys = dsdt_phys_addr + (i * 0x1000);
        uint32_t virt = (uint32_t)ADD_KERNEL_OFFSET(phys);

        pmm_reserve_phys(phys, 0x1000);
        vmm_map_page(virt, phys, PTE_PRESENT | PTE_RW | PDE_PWT);
    }

    struct dsdt *dsdt_table = (struct dsdt *)ADD_KERNEL_OFFSET(dsdt_phys_addr);

    if (strncmp(dsdt_table->header.signature, "DSDT", 4) != 0) {
        KERNEL_PANIC("Failed to find DSDT signature in header", "Invalid DSDT table from FADT");
    }

    return dsdt_table;
}
