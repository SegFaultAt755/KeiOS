/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "drivers/vga.h"
#include "kernel/qemu.h"
#include "libkern/stdio.h"

#define VGA_CURSOR_INDEX 0x3D4
#define VGA_CURSOR_DATA  0x3D5
#define VGA_REG_CURSOR_START 0x0A
#define VGA_REG_CURSOR_END   0x0B
#define VGA_REG_CURSOR_HIGH  0x0E
#define VGA_REG_CURSOR_LOW   0x0F

static volatile uint16_t *text_mem = (volatile uint16_t *)VGA_TEXT_MEMORY;

void vga_init_text(void) {
    qemu_printf(QEMU_DRV, QEMU_OK, "VGA text mode initialized");
}

void vga_clear(enum vga_8b_colors color) {
    for (uint32_t i = 0; i < (VGA_TEXT_WIDTH * VGA_TEXT_HEIGHT); i++) {
        text_mem[i] = vga_entry(' ', vga_entry_color(VGA_8B_WHITE, color));
    }
}

void vga_enable_cursor(uint8_t cursor_start, uint8_t cursor_end) {
    outb(VGA_CURSOR_INDEX, VGA_REG_CURSOR_START);
    outb(VGA_CURSOR_DATA, (inb(VGA_CURSOR_DATA) & 0xC0) | cursor_start);

    outb(VGA_CURSOR_INDEX, VGA_REG_CURSOR_END);
    outb(VGA_CURSOR_DATA, (inb(VGA_CURSOR_DATA) & 0xE0) | cursor_end);
}

void vga_disable_cursor(void) {
    outb(VGA_CURSOR_INDEX, VGA_REG_CURSOR_START);
    outb(VGA_CURSOR_DATA, 0x20);
}

void vga_update_cursor(uint16_t x, uint16_t y) {
    uint16_t pos = y * VGA_TEXT_WIDTH + x;
    outb(VGA_CURSOR_INDEX, VGA_REG_CURSOR_LOW);
    outb(VGA_CURSOR_DATA, (uint8_t)(pos & 0xFF));
    outb(VGA_CURSOR_INDEX, VGA_REG_CURSOR_HIGH);
    outb(VGA_CURSOR_DATA, (uint8_t)((pos >> 8) & 0xFF));
}
