#pragma once

/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include <stdint.h>

enum display_status {
    DISPLAY_OK = 0,
    DISPLAY_ERR_ALREADY_INITIALIZED = 1,
    DISPLAY_ERR_INVALID_ARGUMENT = 2,
    DISPLAY_ERR_NOT_INITIALIZED = 3,
    DISPLAY_ERR_UNSUPPORTED = 4,
};

typedef enum display_status display_status_t;

typedef enum display_text_status {
    DISPLAY_TEXT_OK = 0,
    DISPLAY_TEXT_ERR_INVALID_ARGUMENT = 1,
    DISPLAY_TEXT_ERR_NOT_INITIALIZED = 2,
} display_text_status_t;

/* Basic display operations */
display_status_t display_init(void);
display_status_t display_is_initialized(void);
void display_clear(uint32_t color);
void display_draw_pixel(int32_t x, int32_t y, uint32_t color);
void display_scroll_up(uint32_t height, uint32_t color);

/* Shape drawing operations */
void display_draw_line(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t color);
void display_draw_rect(int32_t x, int32_t y, uint32_t width, uint32_t height, uint32_t color);
void display_fill_rect(int32_t x, int32_t y, uint32_t width, uint32_t height, uint32_t color);

/* Draw text using the default 16x16 ASCII font. */
display_text_status_t display_draw_text_static(int32_t x, int32_t y, const char *text, uint32_t color);
