/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "kernel/acpi.h"
#include "kernel/panic.h"
#include "kernel/qemu.h"
#include "libkern/memory.h"

/**
 * Shutdown the system using ACPI S5 (Soft Off) state.
 * Searches the DSDT AML bytecode for the _S5_ object and extracts sleep values.
 * @param fadt Pointer to the FADT
 * @param dsdt Pointer to the DSDT containing AML bytecode
 */
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
                if (aml[ptr] == 0x0A)
                    ptr++; /* BytePrefix */
                slp_typa = aml[ptr++];

                /* Extract byte value for SLP_TYPb */
                if (aml[ptr] == 0x0A)
                    ptr++; /* BytePrefix */
                slp_typb = aml[ptr++];

                s5_found = true;
                break;
            }
        }
    }

    /* Fallback values for hypervisors like QEMU if _S5_ is missing */
    if (!s5_found) {
        qemu_printf(QEMU_KERN, QEMU_WARN, "Falling back to QEMU default S5 sleep values");
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
