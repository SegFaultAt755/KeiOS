#pragma once

/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include <stdint.h>

/* An interface over raw int color */

struct color32 {
    uint8_t a;
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

static inline struct color32 gen_color32(uint32_t c) {
    return (struct color32){
        .a = (uint8_t)(c >> 24),
        .r = (uint8_t)(c >> 16),
        .g = (uint8_t)(c >> 8),
        .b = c & 0xff
    };
}

static inline uint32_t color32_to_int(struct color32 c) {
    return (c.a << 24) + (c.r << 16) + (c.g << 8) + (c.b);
}
