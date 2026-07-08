#include "drivers/vga.h"
#include "kernel/qemu.h"

static uint16_t *vga_text_buffer = (uint16_t*) VGA_TEXT_MEMORY;
static bool video_mode_enabled = false;

/* Global framebuffer metadata */
static uint32_t *gfx_buffer;
static uint32_t  gfx_pitch;
static uint32_t  gfx_width;
static uint32_t  gfx_height;
static uint8_t   gfx_bpp;

void vga_init_text_mode(void) {
    qemu_printf(QEMU_LOG_INFO, "Initializing VGA text mode", __FILE__, __LINE__);
    video_mode_enabled = false;
    /* Switch to text mode */
}

bool vga_init_graphics(MultibootInfo *boot_info) {
    qemu_printf(QEMU_LOG_INFO, "Initializing VGA video mode", __FILE__, __LINE__);
    qemu_printf(QEMU_LOG_INFO, "Checking if bootloader loaded neccesary flags");
    if (!(boot_info -> flags & (1 << 12))) {
        return false;
    }

    qemu_printf(QEMU_LOG_INFO, "Loading data from multiboot data to local variables");
    gfx_buffer = (uint32_t*)(uintptr_t) boot_info -> framebuffer_addr;
    gfx_pitch  = boot_info -> framebuffer_pitch;
    gfx_width  = boot_info -> framebuffer_width;
    gfx_height = boot_info -> framebuffer_height;
    gfx_bpp    = boot_info -> framebuffer_bpp;

    qemu_printf(QEMU_LOG_INFO, "Enabling video mode and returning");
    video_mode_enabled = true;
    return true;
}

bool vga_is_graphics_supported(void) {
    qemu_printf(QEMU_LOG_INFO, "Getting if video mode is supported {%d}", __FILE__, __LINE__, video_mode_enabled);
    return video_mode_enabled;
}

void vga_clear_screen(uint32_t color) {
    qemu_printf(QEMU_LOG_INFO, "Clearing screen for video and text modes", __FILE__, __LINE__);
    if (video_mode_enabled) {
        qemu_printf(QEMU_LOG_INFO, "Clearing screen for video mode {%dx%d}", gfx_width, gfx_height);
        for (uint32_t y = 0; y < gfx_height; y++) {
            for (uint32_t x = 0; x < gfx_width; x++) {
                vga_set_pixel(x, y, color);
            }
        }
    } else {
        qemu_printf(QEMU_LOG_INFO, "Clearing screen for text mode");
        for (uint32_t i = 0; i < (VGA_TEXT_WIDTH * VGA_TEXT_HEIGHT); i++)
            {vga_text_buffer[i] = vga_entry(' ', vga_entry_color(VGA_COLOR_WHITE, color));}
    }
}

void vga_set_pixel(uint32_t x, uint32_t y, uint32_t color) {
    if (!video_mode_enabled || x >= gfx_width || y >= gfx_height) {
        qemu_printf(QEMU_LOG_ERROR, "(%s:%d) Failed to set pixel: {%d, %d:%d:%d, %d:%d:%d}", __FILE__, __LINE__,
            video_mode_enabled, x, gfx_width, x >= gfx_width, y, gfx_height, y >= gfx_height
        ); return;
    }
    
    /* Calculate byte offset */
    uint8_t *row = (uint8_t*) gfx_buffer + (y * gfx_pitch);
    uint32_t *pixel = (uint32_t*) (row + (x * (gfx_bpp / 8)));

    *pixel = color;
}
