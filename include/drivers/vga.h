#pragma once

#include "kernel/multiboot.h"
#include <stdint.h>

#define VGA_TEXT_WIDTH  80
#define VGA_TEXT_HEIGHT 25
#define VGA_TEXT_MEMORY 0xC00B8000

#define VGA_GFX_WIDTH  640
#define VGA_GFX_HEIGHT 480
#define VGA_GFX_MEMORY 0xC00A0000

enum vga_8b_colors {
    VGA_8B_BLACK         = 0,
    VGA_8B_BLUE          = 1,
    VGA_8B_GREEN         = 2,
    VGA_8B_CYAN          = 3,
    VGA_8B_RED           = 4,
    VGA_8B_MAGENTA       = 5,
    VGA_8B_BROWN         = 6,
    VGA_8B_LIGHT_GREY    = 7,
    VGA_8B_DARK_GREY     = 8,
    VGA_8B_LIGHT_BLUE    = 9,
    VGA_8B_LIGHT_GREEN   = 10,
    VGA_8B_LIGHT_CYAN    = 11,
    VGA_8B_LIGHT_RED     = 12,
    VGA_8B_LIGHT_MAGENTA = 13,
    VGA_8B_LIGHT_BROWN   = 14,
    VGA_8B_WHITE         = 15
};

static inline uint8_t vga_entry_color(enum vga_8b_colors fg, enum vga_8b_colors bg) {
    return fg | (bg << 4);
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t) uc | ((uint16_t) color << 8);
}

void vga_init_text(void);
bool vga_init_gfx(struct multiboot_info *mbi);
bool vga_is_gfx_supported(void);

void vga_clear(uint32_t color);
void vga_set_pixel(uint32_t x, uint32_t y, uint32_t color);
