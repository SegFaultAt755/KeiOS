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

    if (!acpi_map_range((uint32_t)dsdt_phys_addr, sizeof(struct acpi_header))) {
        KERNEL_PANIC("Could not map DSDT header", "DSDT Initialization Failed");
    }

    struct acpi_header *temp_header = (struct acpi_header *)ADD_KERNEL_OFFSET(dsdt_phys_addr);
    if (temp_header->length < sizeof(struct acpi_header)) {
        KERNEL_PANIC("Invalid DSDT length in FADT", "DSDT table is corrupted");
    }

    if (!acpi_map_range((uint32_t)dsdt_phys_addr, temp_header->length)) {
        KERNEL_PANIC("Could not map complete DSDT", "DSDT Initialization Failed");
    }

    struct dsdt *dsdt_table = (struct dsdt *)ADD_KERNEL_OFFSET(dsdt_phys_addr);

    if (strncmp(dsdt_table->header.signature, "DSDT", 4) != 0) {
        KERNEL_PANIC("Failed to find DSDT signature in header", "Invalid DSDT table from FADT");
    }

    return dsdt_table;
}
