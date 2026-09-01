/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "kernel/acpi.h"
#include "kernel/panic.h"
#include "kernel/qemu.h"
#include "kernel/rsdp.h"
#include "libkern/string.h"
#include "libkern/stdio.h"
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

void acpi_shutdown(struct fadt *fadt, struct dsdt *dsdt) {
    if (!fadt || !dsdt) {
        KERNEL_PANIC("ACPI Shutdown Error", "Null FADT or DSDT reference");
    }

    uint32_t aml_len = dsdt->header.length - sizeof(struct acpi_header);
    uint8_t *aml = dsdt->aml_code;

    uint8_t slp_typa = 0;
    uint8_t slp_typb = 0;
    bool s5_found = false;

    /* Search AML bytecode for the "_S5_" object signature */
    for (uint32_t i = 0; i < aml_len - 4; i++) {
        if (memcmp(&aml[i], "_S5_", 4) == 0) {
            uint32_t ptr = i + 4;

            /* Parse AML Package Opcode (0x12) */
            if (aml[ptr] == 0x12) {
                ptr += 2; /* Skip PackageOp and PkgLength */
                ptr++;    /* Skip NumElements */

                /* Extract byte value for SLP_TYPa */
                if (aml[ptr] == 0x0A) ptr++; /* BytePrefix */
                slp_typa = aml[ptr++];

                /* Extract byte value for SLP_TYPb */
                if (aml[ptr] == 0x0A) ptr++; /* BytePrefix */
                slp_typb = aml[ptr++];

                s5_found = true;
                break;
            }
        }
    }

    /* Fallback values for hypervisors like QEMU if _S5_ is missing or default */
    if (!s5_found) {
        qemu_printf(QEMU_KERN, QEMU_WARN, "Triggering QEMU shutdown. Not real one!");
        slp_typa = 5;
        slp_typb = 5;
    }

    /* Shift by 10 bits and set the Sleep Enable (SLP_EN) bit into 16-bit variables */
    uint16_t pm1a_cnt = (slp_typa << 10) | ACPI_SLP_EN;
    uint16_t pm1b_cnt = (slp_typb << 10) | ACPI_SLP_EN;

    /* Issue I/O commands to trigger hardware power-off */
    outw(fadt->pm1a_cnt_blk, pm1a_cnt);
    if (fadt->pm1b_cnt_blk) {
        outw(fadt->pm1b_cnt_blk, pm1b_cnt);
    }
}
