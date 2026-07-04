#include "drivers/vga.h"

uint16_t* vga_buffer = (uint16_t*) VGA_MEMORY;

void vga_clear(uint8_t color) {
    for (uint16_t y = 0; y < VGA_HEIGHT; y++) {
        for (uint16_t x = 0; x < VGA_WIDTH; x++) {
            vga_set_pixel(x, y, color);
        }
    }
}

void vga_set_pixel(uint16_t x, uint16_t y, uint8_t color) {
    vga_buffer[(y * 320) + x] = color;
}