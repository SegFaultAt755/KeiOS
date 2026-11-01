/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "kernel/acpi.h"
#include "kernel/qemu.h"
#include "libkern/memory.h"

static bool aml_read_package_length(const uint8_t *aml, size_t aml_len, size_t *offset, size_t *package_end) {
    size_t length_offset = *offset;
    if (length_offset >= aml_len) {
        return false;
    }

    uint8_t lead = aml[(*offset)++];
    uint8_t follow_bytes = lead >> 6;
    uint32_t package_length = follow_bytes == 0 ? (lead & 0x3FU) : (lead & 0x0FU);

    for (uint8_t i = 0; i < follow_bytes; i++) {
        if (*offset >= aml_len) {
            return false;
        }
        package_length |= (uint32_t)aml[(*offset)++] << (4U + 8U * i);
    }

    size_t encoded_length = *offset - length_offset;
    if (package_length < encoded_length || package_length > aml_len - length_offset) {
        return false;
    }

    *package_end = length_offset + package_length;
    return true;
}

static bool aml_read_integer(const uint8_t *aml, size_t package_end, size_t *offset, uint8_t *value) {
    if (*offset >= package_end) {
        return false;
    }

    uint8_t opcode = aml[(*offset)++];
    uint64_t integer = 0;
    size_t integer_size = 0;
    switch (opcode) {
    case 0x00:
        integer = 0;
        break;
    case 0x01:
        integer = 1;
        break;
    case 0xFF:
        integer = UINT64_MAX;
        break;
    case 0x0A:
        integer_size = 1;
        break;
    case 0x0B:
        integer_size = 2;
        break;
    case 0x0C:
        integer_size = 4;
        break;
    case 0x0E:
        integer_size = 8;
        break;
    default:
        return false;
    }

    if (integer_size > package_end - *offset) {
        return false;
    }
    for (size_t i = 0; i < integer_size; i++) {
        integer |= (uint64_t)aml[(*offset)++] << (8U * i);
    }

    if (integer > 7) {
        return false;
    }
    *value = (uint8_t)integer;
    return true;
}

static bool aml_find_s5(const uint8_t *aml, size_t aml_len, uint8_t *slp_typa, uint8_t *slp_typb) {
    for (size_t i = 0; i <= aml_len && aml_len - i >= 5; i++) {
        if (memcmp(&aml[i], "_S5_", 4) != 0 || aml[i + 4] != 0x12) {
            continue;
        }

        size_t offset = i + 5;
        size_t package_end;
        if (!aml_read_package_length(aml, aml_len, &offset, &package_end) || offset >= package_end) {
            continue;
        }

        uint8_t element_count = aml[offset++];
        if (element_count < 2 || !aml_read_integer(aml, package_end, &offset, slp_typa) ||
            !aml_read_integer(aml, package_end, &offset, slp_typb)) {
            continue;
        }

        return true;
    }

    return false;
}

/**
 * Shutdown the system using ACPI S5 (Soft Off) state.
 * The sleep values are used only when a bounded AML package can be parsed.
 */
void acpi_shutdown(struct fadt *fadt, struct dsdt *dsdt) {
    if (!fadt || !dsdt || dsdt->header.length < sizeof(struct acpi_header)) {
        qemu_printf(QEMU_KERN, QEMU_ERROR, "ACPI shutdown failed: invalid FADT or DSDT");
        return;
    }
    if (fadt->pm1a_cnt_blk > UINT16_MAX || fadt->pm1b_cnt_blk > UINT16_MAX) {
        qemu_printf(QEMU_KERN, QEMU_ERROR, "ACPI shutdown failed: invalid PM1 control port");
        return;
    }

    size_t aml_len = dsdt->header.length - sizeof(struct acpi_header);
    const uint8_t *aml = dsdt->aml_code;
    uint8_t slp_typa;
    uint8_t slp_typb;
    if (!aml_find_s5(aml, aml_len, &slp_typa, &slp_typb)) {
        qemu_printf(QEMU_KERN, QEMU_ERROR, "ACPI shutdown failed: no valid _S5_ package in DSDT");
        return;
    }

    uint16_t pm1a_cnt = ((uint16_t)slp_typa << 10) | ACPI_SLP_EN;
    uint16_t pm1b_cnt = ((uint16_t)slp_typb << 10) | ACPI_SLP_EN;
    outw(fadt->pm1a_cnt_blk, pm1a_cnt);
    if (fadt->pm1b_cnt_blk) {
        outw(fadt->pm1b_cnt_blk, pm1b_cnt);
    }
}
