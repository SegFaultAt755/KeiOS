#pragma once

/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include <stdint.h>

/* Basic display operations */
bool display_init(void);
bool display_is_initialized(void);
void display_clear(uint32_t color);
void display_draw_pixel(int32_t x, int32_t y, uint32_t color);
void display_scroll_up(uint32_t height, uint32_t color);

/* Shape drawing operations */
void display_draw_line(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t color);
void display_draw_rect(int32_t x, int32_t y, uint32_t width, uint32_t height, uint32_t color);
void display_fill_rect(int32_t x, int32_t y, uint32_t width, uint32_t height, uint32_t color);

/* Draw text using the default 16x16 ASCII font; returns 1 for invalid input or an uninitialized display. */
int32_t display_draw_text_static(int32_t x, int32_t y, const char *text, uint32_t color);
