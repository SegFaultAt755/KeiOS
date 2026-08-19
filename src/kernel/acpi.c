/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "kernel/acpi.h"
#include "libkern/string.h"

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

        /* TODO: Map phys_addr to vmem */
        struct acpi_header *table = (struct acpi_header *)phys_addr;

        if (strncmp(table->signature, "FACP", 4) == 0)
            return table;
    }

    return nullptr;
}
