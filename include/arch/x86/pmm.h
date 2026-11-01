#pragma once

/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "kernel/multiboot.h"
#include <stdint.h>

void pmm_init(const struct multiboot_info *mbi, uint32_t physical_alloc_start);
uint32_t pmm_alloc_frame(void);
uint32_t pmm_alloc_contiguous(uint32_t size, uint32_t alignment);
void pmm_reserve_phys(uint64_t phys_addr, uint64_t size);
