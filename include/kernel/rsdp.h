#pragma once

/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include <stdint.h>

#define RSDP_SIGNATURE "RSD PTR "
#define RSDP_SIGNATURE_LEN 8

struct [[gnu::packed]] rsdp {
    char signature[8];
    uint8_t checksum;
    char oem_id[6];
    uint8_t revision;
    uint32_t rsdt_address;
};

struct [[gnu::packed]] rsdp20 {
    struct rsdp first_part;
    uint32_t length;
    uint64_t xsdt_address;
    uint8_t extended_checksum;
    uint8_t reserved[3];
};

uint32_t *find_rsdp_addr(void);
