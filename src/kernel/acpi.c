/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "kernel/acpi.h"
#include "kernel/panic.h"
#include "kernel/qemu.h"
#include "kernel/rsdp.h"
#include "libkern/string.h"
#include "libkern/stdio.h"

#if defined(__i386__) || defined(_M_IX86)
#include "arch/x86/mem.h"
#include "arch/x86/pmm.h"
#include "arch/x86/vmm.h"
#else
#error "Unsupported architecture! (i386 is available)"
#endif

struct acpi_header *find_fadt(struct acpi_header *table, bool is_xsdt) {
    int pointer_size = is_xsdt ? 8 : 4;
    int entries = (table->length - sizeof(struct acpi_header)) / pointer_size;

    uint8_t *pointer_array = (uint8_t *)table + sizeof(struct acpi_header);

    for (int i = 0; i < entries; i++) {
        uintptr_t phys_addr;
        if (is_xsdt)
            phys_addr = *(uint64_t *)(pointer_array + i * 8);
        else
            phys_addr = *(uint32_t *)(pointer_array + i * 4);

        struct acpi_header *table = (struct acpi_header *)ADD_KERNEL_OFFSET(phys_addr);

        if (strncmp(table->signature, "FACP", 4) == 0)
            return table;
    }

    return nullptr;
}

void enable_acpi_mode(struct fadt *fadt) {
    if ((inw(fadt->pm1a_cnt_blk) & 1) == 1) {
        qemu_printf(QEMU_KERN, QEMU_INFO, "ACPI is already enabled by firmware");
        return;
    }

    if (fadt->smi_cmd == 0) {
        qemu_printf(QEMU_KERN, QEMU_WARN, "Firmware doesn\'t support SMI");
        return; 
    }

    outb(fadt->smi_cmd, fadt->acpi_enable);

    /* Wait until the SCI_EN bit is set to 1 */
    while ((inw(fadt->pm1a_cnt_blk) & 1) == 0) {}
}

[[no_discard]]
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
        KERNEL_PANIC("Failed to find FACP signature in root table", "Invalid root table from RSPD");

    return (struct fadt *)fadt_header;
}
