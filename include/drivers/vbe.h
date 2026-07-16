#pragma once

/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include <stdint.h>
#include "kernel/multiboot.h"

#define VBE_VIRTUAL_LFB_START 0xE0000000 

void vbe_initialize(struct multiboot_info *mbi);
void vbe_set_pixel(int x, int y, uint32_t color);
