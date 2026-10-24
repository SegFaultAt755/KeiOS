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
    uintptr_t root_phys_addr = 0;

    if (is_xsdt) {
        struct rsdp20 *rsdp20 = (struct rsdp20 *)rsdp;
        root_phys_addr = (uintptr_t)rsdp20->xsdt_address;
    } else {
        root_phys_addr = (uintptr_t)rsdp->rsdt_address;
    }

    if (root_phys_addr == 0) {
        KERNEL_PANIC("Missing ACPI root table", "RSDP root table address is null");
    }

    if (!acpi_map_range((uint32_t)root_phys_addr, sizeof(struct acpi_header))) {
        KERNEL_PANIC("Could not map ACPI root-table header", "ACPI initialization failed");
    }

    struct acpi_header *root_table = (struct acpi_header *)ADD_KERNEL_OFFSET(root_phys_addr);
    if (root_table->length < sizeof(struct acpi_header)) {
        KERNEL_PANIC("ACPI root table length is invalid", "RSDP root table is corrupted");
    }

    if (!acpi_map_range((uint32_t)root_phys_addr, root_table->length)) {
        KERNEL_PANIC("Could not map complete ACPI root table", "ACPI initialization failed");
    }

    root_table = (struct acpi_header *)ADD_KERNEL_OFFSET(root_phys_addr);
    struct acpi_header *fadt_header = find_fadt(root_table, is_xsdt);
    if (!fadt_header)
        KERNEL_PANIC("Failed to find FACP signature in root table", "Invalid root table from RSDP");

    return (struct fadt *)fadt_header;
}
