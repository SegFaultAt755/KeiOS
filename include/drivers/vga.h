#pragma once

#include "kernel/multiboot.h"
#include <stdint.h>

#define VGA_TEXT_WIDTH  80
#define VGA_TEXT_HEIGHT 25
#define VGA_TEXT_MEMORY 0xC00B8000

#define VGA_GFX_WIDTH  320
#define VGA_GFX_HEIGHT 200
#define VGA_GFX_MEMORY 0xC00A0000

typedef enum VgaColors {
    VGA_COLOR_BLACK         = 0,
    VGA_COLOR_BLUE          = 1,
    VGA_COLOR_GREEN         = 2,
    VGA_COLOR_CYAN          = 3,
    VGA_COLOR_RED           = 4,
    VGA_COLOR_MAGENTA       = 5,
    VGA_COLOR_BROWN         = 6,
    VGA_COLOR_LIGHT_GREY    = 7,
    VGA_COLOR_DARK_GREY     = 8,
    VGA_COLOR_LIGHT_BLUE    = 9,
    VGA_COLOR_LIGHT_GREEN   = 10,
    VGA_COLOR_LIGHT_CYAN    = 11,
    VGA_COLOR_LIGHT_RED     = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN   = 14,
    VGA_COLOR_WHITE         = 15
} VgaColors;

static inline uint8_t vga_entry_color(VgaColors fg, VgaColors bg) {
    return fg | (bg << 4);
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t) uc | ((uint16_t) color << 8);
}

void vga_init_text_mode(void);
bool vga_init_graphics(MultibootInfo *boot_info);
bool vga_is_graphics_supported(void);

void vga_clear_screen(uint32_t color);
void vga_set_pixel(uint32_t x, uint32_t y, uint32_t color);
