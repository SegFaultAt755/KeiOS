#pragma once

/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include <stdint.h>

uint32_t vmm_initialize(uint32_t mem_high_point, uint32_t physical_alloc_start);
bool vmm_map_page(uint32_t virt_addr, uint32_t phys_addr, uint32_t flags);
void vmm_unmap_page(uint32_t virt_addr);
uint32_t vmm_get_phys(uint32_t virt_addr);
