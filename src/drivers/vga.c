#include "drivers/vga.h"
#include "kernel/qemu.h"

static uint16_t *text_mem = (uint16_t*) VGA_TEXT_MEMORY;
static bool video_mode_enabled = false;

static uint32_t *video_mem;
static uint32_t video_width;
static uint32_t video_height;
static uint32_t video_pitch;
static uint8_t video_bpp;

void vga_init_text_mode(void) {
    qemu_printf(QEMU_LOG_INFO, "Initializing VGA text mode");
    video_mode_enabled = false;
    /* Switch to text mode */
}

bool vga_init_graphics(struct multiboot_info *mbi) {
    qemu_printf(QEMU_LOG_INFO, "Initializing VGA video mode");
    qemu_printf(QEMU_LOG_INFO, "Checking if bootloader loaded neccesary flags");
    if (!(mbi -> flags & (1 << 12)))
        return false;

    qemu_printf(QEMU_LOG_INFO, "Loading data from multiboot data to local variables");
    video_mem = (uint32_t *)(uintptr_t) mbi -> framebuffer_addr;
    video_width = mbi -> framebuffer_width;
    video_height = mbi -> framebuffer_height;
    video_pitch = mbi -> framebuffer_pitch;
    video_bpp = mbi -> framebuffer_bpp;

    qemu_printf(QEMU_LOG_INFO, "Enabling video mode and returning");
    video_mode_enabled = true;
    return true;
}

bool vga_is_graphics_supported(void) {
    return video_mode_enabled;
}

void vga_clear_screen(uint32_t color) {
    qemu_printf(QEMU_LOG_INFO, "Clearing screen for video and text modes");
    if (video_mode_enabled) {
        qemu_printf(QEMU_LOG_INFO, "Clearing screen for video mode {%dx%d}", video_width, video_height);
        for (uint32_t y = 0; y < video_height; y++) {
            for (uint32_t x = 0; x < video_width; x++) {
                vga_set_pixel(x, y, color);
            }
        }
    } else {
        qemu_printf(QEMU_LOG_INFO, "Clearing screen for text mode");
        for (uint32_t i = 0; i < (VGA_TEXT_WIDTH * VGA_TEXT_HEIGHT); i++)
            {text_mem[i] = vga_entry(' ', vga_entry_color(VGA_COLOR_WHITE, color));}
    }
}

void vga_set_pixel(uint32_t x, uint32_t y, uint32_t color) {
    if (!video_mode_enabled || x >= video_width || y >= video_height) {
        qemu_printf(QEMU_LOG_ERROR, "(%s:%d) Failed to set pixel: {%d, %d:%d:%d, %d:%d:%d}", __FILE__, __LINE__,
            video_mode_enabled, x, video_width, x >= video_width, y, video_height, y >= video_height
        ); return;
    }
    
    /* Calculate byte offset */
    uint8_t *row = (uint8_t*) video_mem + (y * video_pitch);
    uint32_t *pixel = (uint32_t*) (row + (x * (video_bpp / 8)));

    *pixel = color;
}
