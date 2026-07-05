#include "drivers/vga.h"

static uint16_t *vga_text_buffer = (uint16_t*) VGA_TEXT_MEMORY;
static uint8_t  *vga_gfx_buffer  = (uint8_t*)  VGA_GFX_MEMORY;
static bool graphics_mode_enabled = false;

void vga_init_text_mode(void) {
    graphics_mode_enabled = false;
    /* Switch to text mode */
}

bool vga_init_graphics(void) {
    /* Switch to video mode, return false if failed */
    graphics_mode_enabled = true;
    return true;
}

bool vga_is_graphics_supported(void) {
    return graphics_mode_enabled;
}

void vga_clear_screen(uint8_t color) {
    if (graphics_mode_enabled) {
        for (uint32_t i = 0; i < (VGA_GFX_WIDTH * VGA_GFX_HEIGHT); i++)
            {vga_gfx_buffer[i] = color;}
    } else {
        for (uint32_t i = 0; i < (VGA_TEXT_WIDTH * VGA_TEXT_HEIGHT); i++)
            {vga_text_buffer[i] = vga_entry(' ', vga_entry_color(VGA_COLOR_WHITE, color));}
    }
}

void vga_set_pixel(uint16_t x, uint16_t y, uint8_t color) {
    if (!graphics_mode_enabled || x >= VGA_GFX_WIDTH || y >= VGA_GFX_HEIGHT)
        return; 

    vga_gfx_buffer[(y * VGA_GFX_WIDTH) + x] = color;
}
