#pragma once

/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include <stdbool.h>
#include <stdint.h>

struct multiboot_info;

int framebuffer_init(const struct multiboot_info *mbi);
void framebuffer_clear(uint32_t color);
void framebuffer_draw_pixel(uint32_t color, uint32_t x, uint32_t y);
void framebuffer_fill_rect(uint32_t color, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
void framebuffer_scroll_up(uint32_t height, uint32_t color);
