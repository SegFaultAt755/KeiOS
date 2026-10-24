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

    uint8_t slp_typa = 0;
    uint8_t slp_typb = 0;
    bool s5_found = false;

    if (dsdt->header.length < sizeof(struct acpi_header)) {
        qemu_printf(QEMU_KERN, QEMU_WARN, "Malformed DSDT length; falling back to QEMU default S5 sleep values");
        slp_typa = 5;
        slp_typb = 5;
    } else {
        uint32_t aml_len = dsdt->header.length - sizeof(struct acpi_header);
        uint8_t *aml = dsdt->aml_code;

        /* Search AML bytecode for the "_S5_" object signature */
        for (uint32_t i = 0; i + 4 <= aml_len; i++) {
            if (memcmp(&aml[i], "_S5_", 4) != 0) {
                continue;
            }

            uint32_t ptr = i + 4;
            if (ptr >= aml_len || aml[ptr] != 0x12) {
                continue;
            }

            ptr += 1; /* PackageOp */
            if (ptr >= aml_len) {
                break;
            }

            uint8_t pkg_len_prefix = aml[ptr++];
            if ((pkg_len_prefix & 0x80U) != 0U) {
                uint8_t extra_bytes = pkg_len_prefix & 0x07U;
                if (ptr + extra_bytes > aml_len) {
                    break;
                }
                ptr += extra_bytes;
            }

            if (ptr >= aml_len) {
                break;
            }

            uint8_t num_elements = aml[ptr++];
            if (num_elements == 0 || ptr + 1 >= aml_len) {
                break;
            }

            if (aml[ptr] == 0x0A) {
                ptr++;
            }
            if (ptr >= aml_len) {
                break;
            }
            slp_typa = aml[ptr++];

            if (ptr >= aml_len) {
                break;
            }
            if (aml[ptr] == 0x0A) {
                ptr++;
            }
            if (ptr >= aml_len) {
                break;
            }
            slp_typb = aml[ptr++];

            s5_found = true;
            break;
        }

        if (!s5_found) {
            qemu_printf(QEMU_KERN, QEMU_WARN, "Falling back to QEMU default S5 sleep values");
            slp_typa = 5;
            slp_typb = 5;
        }
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
