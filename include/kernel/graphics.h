#pragma once

/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "kernel/multiboot.h"
#include "drivers/graphics/color.h"

#define GRAPHICS_TYPE_VGA_PALETTE 2
#define GRAPHICS_TYPE_FRAMEBUFFER 1
#define GRAPHICS_TYPE_TEXT_MODE   0

extern bool gfx_initialized;

int get_graphics_type(struct multiboot_info *mbi);
int gfx_init(struct multiboot_info *mbi);
void gfx_set_ipc_endpoint(uint32_t endpoint);
int gfx_clear(struct color32 color);
int gfx_draw_pixel(struct color32 color, uint32_t x, uint32_t y);
int gfx_fill_rect(struct color32 color, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
