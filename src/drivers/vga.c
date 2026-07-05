#include "drivers/vga.h"

static uint16_t *vga_text_buffer = (uint16_t*) VGA_TEXT_MEMORY;
static bool video_mode_enabled = false;

/* Global framebuffer metadata */
static uint32_t *gfx_buffer;
static uint32_t  gfx_pitch;
static uint32_t  gfx_width;
static uint32_t  gfx_height;
static uint8_t   gfx_bpp;

void vga_init_text_mode(void) {
    video_mode_enabled = false;
    /* Switch to text mode */
}

bool vga_init_graphics(MultibootInfo *boot_info) {
    if (!(boot_info -> flags & (1 << 12))) {
        return false;
    }

    gfx_buffer = (uint32_t*)(uintptr_t) boot_info -> framebuffer_addr;
    gfx_pitch  = boot_info -> framebuffer_pitch;
    gfx_width  = boot_info -> framebuffer_width;
    gfx_height = boot_info -> framebuffer_height;
    gfx_bpp    = boot_info -> framebuffer_bpp;

    video_mode_enabled = true;
    return true;
}

bool vga_is_graphics_supported(void) {
    return video_mode_enabled;
}

void vga_clear_screen(uint32_t color) {
    if (video_mode_enabled) {
        for (uint32_t y = 0; y < gfx_height; y++)
            for (uint32_t x = 0; x < gfx_width; x++)
                vga_set_pixel(x, y, color);
    } else {
        for (uint32_t i = 0; i < (VGA_TEXT_WIDTH * VGA_TEXT_HEIGHT); i++)
            {vga_text_buffer[i] = vga_entry(' ', vga_entry_color(VGA_COLOR_WHITE, color));}
    }
}

void vga_set_pixel(uint32_t x, uint32_t y, uint32_t color) {
    if (!video_mode_enabled || x >= gfx_width || y >= gfx_height) return;
    
    /* Calculate byte offset */
    uint8_t *row = (uint8_t*) gfx_buffer + (y * gfx_pitch);
    uint32_t *pixel = (uint32_t*) (row + (x * (gfx_bpp / 8)));
    
    *pixel = color;
}
