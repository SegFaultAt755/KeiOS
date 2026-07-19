#pragma once

/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include <stdint.h>
#include "kernel/multiboot.h"

#define VBE_VIRTUAL_LFB_START 0xE0000000

struct vbe_info {
    uint32_t *lfb_addr;
    uint32_t flags;
    uint32_t width, height, pitch;
    uint8_t bpp;
};

void vbe_initialize(struct vbe_info info);
void vbe_set_pixel(int x, int y, uint32_t color);
uint32_t *vbe_get_lfb_addr(void);
uint32_t  vbe_get_width(void);
uint32_t  vbe_get_height(void);
uint32_t  vbe_get_pitch(void);
uint8_t   vbe_get_bpp(void);
