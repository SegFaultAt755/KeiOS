/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "kernel/rsdp.h"
#include "kernel/qemu.h"
#include "libkern/memory.h"
#include <stdint.h>

uint32_t *find_rsdp_addr(void) {
    uintptr_t start_addr = 0x000E0000;
    uintptr_t end_addr = 0x000FFFFF;

    for (uintptr_t addr = start_addr; addr < end_addr; addr += 16) {
        char *p = (char *)addr;

        if (memcmp(p, RSDP_SIGNATURE, RSDP_SIGNATURE_LEN) == 0) {
            /* Verify checksum before accepting the structure */
            uint8_t sum = 0;
            for (size_t i = 0; i < 20; i++)
                sum += ((uint8_t *)p)[i];

            if (sum == 0)
                return (uint32_t *)addr;
        }
    }

    return nullptr;
}

void parse_rsdp(uint32_t *rsdp_addr) {
    if (rsdp_addr == nullptr)
        return;

    struct rsdp *rsdp = (struct rsdp *)rsdp_addr;

    if (rsdp->revision == 0) {
        uint32_t rsdt_phys_addr = rsdp->rsdt_address;
        qemu_printf(QEMU_KERN, QEMU_OK, "[RSDP] RSDT start address found at %p", rsdt_phys_addr);
        /* TODO: Map rsdt_phys_addr to vmem and parse RSDT */
        return;
    }

    if (rsdp->revision >= 2) {
        struct rsdp20 *rsdp20 = (struct rsdp20 *)rsdp_addr;

        uint8_t ext_sum = 0;
        uint8_t *p = (uint8_t *)rsdp20;
        for (size_t i = 0; i < rsdp20->length; i++)
            ext_sum += p[i];

        if (ext_sum == 0) {
            uint64_t xsdt_phys_addr = rsdp20->xsdt_address;
            /* TODO: Map xsdt_phys_addr to vmem and parse XSDT */
        }
    }
}
