#pragma once

/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include <stdint.h>

void pmm_initialize(uint64_t mem_high_point, uint32_t physical_alloc_start);
uint32_t pmm_alloc_frame(void);
void pmm_free_frame(uint32_t frame_addr);
