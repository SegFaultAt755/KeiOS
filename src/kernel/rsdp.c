/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "kernel/rsdp.h"
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
