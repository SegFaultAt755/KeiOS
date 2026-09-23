#pragma once

/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include <stdint.h>
#include "drivers/graphics/color.h"

/* These values are pulled from multiboot info */
struct framebuffer_info {
    uint32_t *addr;
    uint32_t  flags;
    uint32_t  width;
    uint32_t  height;
    uint32_t  pitch;
    uint8_t   bpp;
};

int framebuffer_init(struct framebuffer_info info);
void framebuffer_clear(struct color32 c);
void framebuffer_draw_pixel(struct color32 c, uint32_t x, uint32_t y);
