#pragma once

/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include <stdint.h>

#define RSDP_SIGNATURE "RSD PTR "
#define RSDP_SIGNATURE_LEN 8

uintptr_t find_rsdp_addr(void);
