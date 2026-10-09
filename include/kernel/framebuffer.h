#pragma once

/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include <stdint.h>

#include "drivers/graphics/color.h"
#include "drivers/graphics/framebuffer.h"

struct display_info {
    uint32_t *lfb_addr;
    uint32_t  flags;
    uint32_t  width;
    uint32_t  height;
    uint32_t  pitch;
    uint8_t   bpp;
};

static inline int display_init(struct display_info info) {
    struct framebuffer_info fb = {
        .addr = info.lfb_addr,
        .flags = info.flags,
        .width = info.width,
        .height = info.height,
        .pitch = info.pitch,
        .bpp = info.bpp,
    };
    return framebuffer_init(fb);
}

static inline void display_clear(uint32_t color) {
    framebuffer_clear(gen_color32(color));
}

static inline void display_draw_pixel(int32_t x, int32_t y, uint32_t color) {
    framebuffer_draw_pixel(gen_color32(color), (uint32_t)x, (uint32_t)y);
}

static inline void display_draw_line(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t color) {
    (void)x0;
    (void)y0;
    (void)x1;
    (void)y1;
    (void)color;
}

static inline void display_draw_rect(int32_t x, int32_t y, uint32_t width, uint32_t height, uint32_t color) {
    (void)x;
    (void)y;
    (void)width;
    (void)height;
    (void)color;
}

static inline void display_fill_rect(int32_t x, int32_t y, uint32_t width, uint32_t height, uint32_t color) {
    framebuffer_fill_rect(gen_color32(color), (uint32_t)x, (uint32_t)y, width, height);
}

static inline uint32_t display_get_width(void) { return 0; }
static inline uint32_t display_get_height(void) { return 0; }
static inline uint32_t display_get_pitch(void) { return 0; }
static inline uint8_t display_get_bpp(void) { return 0; }
static inline uint32_t display_get_flags(void) { return 0; }
static inline uint32_t *display_get_lfb_addr(void) { return 0; }
static inline int32_t display_get_pixel(int32_t x, int32_t y, uint32_t *out_color) {
    (void)x;
    (void)y;
    if (out_color != 0) {
        *out_color = 0;
    }
    return 1;
}
static inline void display_set_lfb_addr(uint32_t *new_lfb) {
    (void)new_lfb;
}
static inline void display_set_resolution(uint32_t width, uint32_t height, uint32_t pitch) {
    (void)width;
    (void)height;
    (void)pitch;
}
