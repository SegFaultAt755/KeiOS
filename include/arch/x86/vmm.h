#pragma once

/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "kernel/multiboot.h"
#include <stdint.h>

void vmm_init(const struct multiboot_info *mbi, uint32_t physical_alloc_start);
bool vmm_map_page(uint32_t virt_addr, uint32_t phys_addr, uint32_t flags);
bool vmm_is_page_mapped(uint32_t virt_addr);
