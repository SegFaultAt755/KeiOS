/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "drivers/vga.h"

#include "kernel/qemu.h"

static uint16_t *text_mem = (uint16_t *)VGA_TEXT_MEMORY;
static bool gfx_mode_enabled = false;

static uint32_t *gfx_mem;
static uint32_t gfx_width;
static uint32_t gfx_height;
static uint32_t gfx_pitch;
static uint8_t gfx_bpp;

void vga_init_text(void) {
    gfx_mode_enabled = false;
    /* Switch to text mode */

    qemu_printf(QEMU_DRV, QEMU_OK, "VGA text mode initialized");
}

bool vga_init_gfx(struct multiboot_info *mbi) {
    if (!(mbi->flags & (1 << 12))) {
        qemu_printf(QEMU_DRV, QEMU_ERROR, "Failed to initialize GFX (flags: %d)", mbi->flags);
        return false;
    }

    gfx_mem = (uint32_t *)(uintptr_t)mbi->framebuffer_addr;
    gfx_width = mbi->framebuffer_width;
    gfx_height = mbi->framebuffer_height;
    gfx_pitch = mbi->framebuffer_pitch;
    gfx_bpp = mbi->framebuffer_bpp;
    gfx_mode_enabled = true;

    qemu_printf(QEMU_DRV, QEMU_OK, "GFX initialized (memory: 0x%x, width: %d, height: %d, pitch: %d, bpp: %d)", gfx_mem,
                gfx_width, gfx_height, gfx_pitch, gfx_bpp);

    return true;
}

bool vga_is_gfx_supported(void) {
    return gfx_mode_enabled;
}

void vga_clear(uint32_t color) {
    if (gfx_mode_enabled) {
        for (uint32_t y = 0; y < gfx_height; y++)
            for (uint32_t x = 0; x < gfx_width; x++)
                vga_set_pixel(x, y, color);
    } else {
        for (uint32_t i = 0; i < (VGA_TEXT_WIDTH * VGA_TEXT_HEIGHT); i++)
            text_mem[i] = vga_entry(' ', vga_entry_color(VGA_8B_WHITE, color));
    }
}

void vga_set_pixel(uint32_t x, uint32_t y, uint32_t color) {
    if (!gfx_mode_enabled || x >= gfx_width || y >= gfx_height) {
        qemu_printf(QEMU_DRV, QEMU_ERROR, "GFX failed to set pixel: {%d, %d:%d:%d, %d:%d:%d}", gfx_mode_enabled, x,
                    gfx_width, x >= gfx_width, y, gfx_height, y >= gfx_height);
        return;
    }

    /* Calculate byte offset */
    uint8_t *row = (uint8_t *)gfx_mem + (y * gfx_pitch);
    uint32_t *pixel = (uint32_t *)(row + (x * (gfx_bpp / 8)));

    *pixel = color;
}
