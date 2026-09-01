/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "kernel/acpi.h"
#include "kernel/panic.h"
#include "kernel/qemu.h"
#include "kernel/rsdp.h"

#if defined(__i386__) || defined(_M_IX86)
#include "arch/x86/mem.h"
#else
#error "Unsupported architecture! (i386 is available)"
#endif

/**
 * Find and return the virtual address of the RSDP.
 * Panics if RSDP is not found.
 */
struct rsdp *boot_init_rsdp(void) {
    uint32_t *rsdp_addr = find_rsdp_addr();
    if (rsdp_addr == nullptr)
        KERNEL_PANIC("Failed to find RSDP start address",
                     "Incompatible device or BIOS failed to load data into memory");

    qemu_printf(QEMU_KERN, QEMU_OK, "RSDP start address found at %p", rsdp_addr);
    return (struct rsdp *)ADD_KERNEL_OFFSET((uintptr_t)rsdp_addr);
}

/**
 * Initialize ACPI subsystem (FADT and DSDT).
 * Must be called after memory management is initialized.
 */
void boot_init_acpi_system(struct rsdp *rsdp) {
    KERNEL_ASSERT(rsdp != nullptr, "Invalid RSDP pointer",
                  "boot_init_acpi_system called with NULL RSDP - RSDP initialization failed");

    global_fadt = acpi_init(rsdp);

    KERNEL_ASSERT(global_fadt != nullptr, "FADT initialization failed",
                  "acpi_init returned NULL FADT - ACPI table parsing failed or FADT not found");

    global_dsdt = parse_dsdt(global_fadt);

    KERNEL_ASSERT(global_dsdt != nullptr, "DSDT parsing failed",
                  "parse_dsdt returned NULL DSDT - ACPI DSDT table not found or corrupted");

    enable_acpi_mode(global_fadt);

    qemu_printf(QEMU_KERN, QEMU_INFO, "[FADT] Power management: pm1a_cnt_blk=0x%x pm1b_cnt_blk=0x%x",
                global_fadt->pm1a_cnt_blk, global_fadt->pm1b_cnt_blk);
}
