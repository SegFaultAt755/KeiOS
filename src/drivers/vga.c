/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "drivers/vga.h"
#include "kernel/qemu.h"

static volatile uint16_t *text_mem = (volatile uint16_t *)VGA_TEXT_MEMORY;

void vga_init_text(void) {
    qemu_printf(QEMU_DRV, QEMU_OK, "VGA text mode initialized");
}

void vga_clear(enum vga_8b_colors color) {
    for (uint32_t i = 0; i < (VGA_TEXT_WIDTH * VGA_TEXT_HEIGHT); i++) {
        text_mem[i] = vga_entry(' ', vga_entry_color(VGA_8B_WHITE, color));
    }
}
