/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "kernel/acpi.h"
#include "kernel/panic.h"
#include "kernel/qemu.h"
#include "libkern/memory.h"
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

    size_t pointer_size = is_xsdt ? sizeof(uint64_t) : sizeof(uint32_t);
    size_t pointer_bytes = table->length - sizeof(struct acpi_header);
    if (pointer_bytes % pointer_size != 0) {
        return nullptr;
    }
    size_t entries = pointer_bytes / pointer_size;

    uint8_t *pointer_array = (uint8_t *)table + sizeof(struct acpi_header);
    for (size_t i = 0; i < entries; i++) {
        uint64_t phys_addr = 0;
        if (is_xsdt) {
            memcpy(&phys_addr, pointer_array + i * pointer_size, sizeof(uint64_t));
        } else {
            uint32_t rsdt_phys_addr;
            memcpy(&rsdt_phys_addr, pointer_array + i * pointer_size, sizeof(rsdt_phys_addr));
            phys_addr = rsdt_phys_addr;
        }

        if (phys_addr == 0 || phys_addr > UINT32_MAX) {
            continue;
        }

        struct acpi_header *entry = acpi_map_range((uint32_t)phys_addr, sizeof(struct acpi_header));
        if (!entry) {
            continue;
        }

        if (entry->length < sizeof(struct acpi_header)) {
            continue;
        }

        if (strncmp(entry->signature, "FACP", 4) != 0) {
            continue;
        }

        entry = acpi_map_range((uint32_t)phys_addr, entry->length);
        if (entry) {
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
