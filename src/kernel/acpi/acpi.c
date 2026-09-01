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
    /* Determine whether ACPI uses an XSDT or an RSDT */
    bool is_xsdt = (rsdp->revision >= 2);
    uintptr_t root_phys_addr = 0;

    if (is_xsdt) {
        struct rsdp20 *rsdp20 = (struct rsdp20 *)rsdp;
        root_phys_addr = (uintptr_t)rsdp20->xsdt_address;
    } else {
        root_phys_addr = (uintptr_t)rsdp->rsdt_address;
    }

    /* Reserve physical frame and map physical address into kernel virtual memory */
    pmm_reserve_phys(root_phys_addr, sizeof(struct acpi_header));
    vmm_map_page((uint32_t)ADD_KERNEL_OFFSET(root_phys_addr), (uint32_t)root_phys_addr, PTE_PRESENT | PTE_RW | PDE_PWT);

    /* Map the ACPI root table into virtual memory */
    struct acpi_header *root_table = (struct acpi_header *)ADD_KERNEL_OFFSET(root_phys_addr);
    struct acpi_header *fadt_header = find_fadt(root_table, is_xsdt);
    if (!fadt_header)
        KERNEL_PANIC("Failed to find FACP signature in root table", "Invalid root table from RSDP");

    return (struct fadt *)fadt_header;
}
