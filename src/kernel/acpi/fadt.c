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
    if (!table || table->length < sizeof(struct acpi_header)) {
        return nullptr;
    }

    int pointer_size = is_xsdt ? 8 : 4;
    int entries = (table->length - sizeof(struct acpi_header)) / pointer_size;

    uint8_t *pointer_array = (uint8_t *)table + sizeof(struct acpi_header);

    for (int i = 0; i < entries; i++) {
        uintptr_t phys_addr = 0;
        if (is_xsdt) {
            phys_addr = *(uint64_t *)(pointer_array + i * 8);
        } else {
            phys_addr = *(uint32_t *)(pointer_array + i * 4);
        }

        if (phys_addr == 0 || phys_addr > UINT32_MAX) {
            continue;
        }

        if (!acpi_map_range((uint32_t)phys_addr, sizeof(struct acpi_header))) {
            continue;
        }

        struct acpi_header *entry = (struct acpi_header *)ADD_KERNEL_OFFSET(phys_addr);
        if (entry->length < sizeof(struct acpi_header)) {
            continue;
        }

        if (!acpi_map_range((uint32_t)phys_addr, entry->length)) {
            continue;
        }

        entry = (struct acpi_header *)ADD_KERNEL_OFFSET(phys_addr);
        if (strncmp(entry->signature, "FACP", 4) == 0) {
            return entry;
        }
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
    if (!fadt) {
        qemu_printf(QEMU_KERN, QEMU_ERROR, "ACPI mode enable failed: null FADT");
        return;
    }

    if (fadt->pm1a_cnt_blk == 0) {
        qemu_printf(QEMU_KERN, QEMU_WARN, "PM1a control block is unavailable; ACPI mode cannot be enabled");
        return;
    }

    if ((inw(fadt->pm1a_cnt_blk) & 1) == 1) {
        qemu_printf(QEMU_KERN, QEMU_INFO, "ACPI is already enabled by firmware");
        return;
    }

    if (fadt->smi_cmd == 0) {
        qemu_printf(QEMU_KERN, QEMU_WARN, "Firmware doesn't support SMI");
        return;
    }

    outb(fadt->smi_cmd, fadt->acpi_enable);

    /* Wait until the SCI_EN bit is set to 1, but fail rather than spin forever. */
    for (uint32_t timeout = 1000000; (inw(fadt->pm1a_cnt_blk) & 1) == 0 && timeout > 0; --timeout) {
    }

    if ((inw(fadt->pm1a_cnt_blk) & 1) == 0) {
        qemu_printf(QEMU_KERN, QEMU_ERROR, "ACPI mode enable timed out after 1,000,000 polls");
    }
}
