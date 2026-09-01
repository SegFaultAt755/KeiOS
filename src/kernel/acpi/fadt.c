/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "kernel/acpi.h"
#include "kernel/panic.h"
#include "kernel/qemu.h"
#include "libkern/string.h"

#if defined(__i386__) || defined(_M_IX86)
#include "arch/x86/mem.h"
#else
#error "Unsupported architecture! (i386 is available)"
#endif

/**
 * Find the FADT (Fixed ACPI Description Table) in the RSDT/XSDT.
 * @param table Root table (RSDT or XSDT)
 * @param is_xsdt True if table is XSDT, false if RSDT
 * @return Pointer to FADT, or nullptr if not found
 */
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

/* ============================================================================
 * ACPI Mode Control
 * ============================================================================ */

/**
 * Enable ACPI mode by setting the SCI_EN bit via SMI.
 * This transfers hardware control from the legacy BIOS to ACPI.
 * @param fadt Pointer to the FADT
 */
void enable_acpi_mode(struct fadt *fadt) {
    if ((inw(fadt->pm1a_cnt_blk) & 1) == 1) {
        qemu_printf(QEMU_KERN, QEMU_INFO, "ACPI is already enabled by firmware");
        return;
    }

    if (fadt->smi_cmd == 0) {
        qemu_printf(QEMU_KERN, QEMU_WARN, "Firmware doesn't support SMI");
        return;
    }

    outb(fadt->smi_cmd, fadt->acpi_enable);

    /* Wait until the SCI_EN bit is set to 1 */
    while ((inw(fadt->pm1a_cnt_blk) & 1) == 0) {
    }
}
