/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "drivers/terminal.h"

#include "kernel/qemu.h"
#include "libkern/string.h"

static uint16_t term_row;
static uint16_t term_column;
static uint8_t term_color;
static uint16_t *term_mem = nullptr;

static uint16_t term_width;
static uint16_t term_height;

void terminal_initialize(uint16_t *mem, uint16_t width, uint16_t height) {
    term_mem = mem;
    term_width = width;
    term_height = height;
    term_color = vga_entry_color(TERMINAL_DEFAULT_FG, TERMINAL_DEFAULT_BG);

    terminal_clear();
    vga_enable_cursor(14, 15);
    vga_update_cursor(term_column, term_row);

    qemu_printf(QEMU_DRV, QEMU_OK, "Terminal initialized (memory: %p, width: %u, height: %u, color: %p)", term_mem,
                (uint32_t)term_width, (uint32_t)term_height, term_color);
}

void terminal_clear() {
    term_row = 0;
    term_column = 0;
    for (auto y = 0u; y < term_height; y++)
        for (auto x = 0u; x < term_width; x++)
            terminal_put_entry(' ', term_color, x, y);
    vga_update_cursor(term_column, term_row);
}

void terminal_set_color(uint8_t color) {
    term_color = color;
}

void terminal_put_entry(char c, uint8_t color, uint16_t x, uint16_t y) {
    if (x < term_width && y < term_height) {
        auto i = (uint32_t)y * term_width + x;
        term_mem[i] = vga_entry(c, color);
    }
}

void terminal_putchar(char c) {
    if (c == '\n') {
        terminal_blankline();
        return;
    }

    if (c == '\t') {
        term_column += TERMINAL_TAB_SIZE - (term_column % TERMINAL_TAB_SIZE);
        if (term_column >= term_width) {
            term_column = 0;
            if (++term_row >= term_height)
                terminal_scroll();
        }

        vga_update_cursor(term_column, term_row);
        return;
    }

    if (c == '\b') {
        if (term_column > 0) {
            term_column--;
        } else if (term_row > 0) {
            term_row--;
            term_column = term_width - 1;
        }

        terminal_put_entry(' ', term_color, term_column, term_row);
        vga_update_cursor(term_column, term_row);
        return;
    }

    terminal_put_entry(c, term_color, term_column, term_row);

    if (++term_column >= term_width) {
        term_column = 0;
        if (++term_row >= term_height)
            terminal_scroll();
    }

    vga_update_cursor(term_column, term_row);
}

void terminal_write(const char *s, size_t size) {
    for (auto i = 0u; i < size; i++)
        terminal_putchar(s[i]);
}

void terminal_writestring(const char *s) {
    for (auto i = 0u; s[i] != '\0'; i++)
        terminal_putchar(s[i]);
}

void terminal_blankline() {
    term_column = 0;
    if (++term_row >= term_height)
        terminal_scroll();
    vga_update_cursor(term_column, term_row);
}

void terminal_scroll() {
    for (auto y = 0u; y < (uint16_t)(term_height - 1); y++)
        for (auto x = 0u; x < term_width; x++)
            term_mem[y * term_width + x] = term_mem[(y + 1) * term_width + x];

    auto l_row = term_height - 1;
    for (auto x = 0u; x < term_width; x++)
        term_mem[l_row * term_width + x] = vga_entry(' ', term_color);

    term_row = l_row;
    vga_update_cursor(term_column, term_row);
}

void terminal_cursor_up() {
    if (term_row > 0)
        term_row--;
    vga_update_cursor(term_column, term_row);
}

void terminal_cursor_down() {
    if (term_row < term_height - 1)
        term_row++;
    vga_update_cursor(term_column, term_row);
}

void terminal_cursor_left() {
    if (term_column > 0)
        term_column--;
    vga_update_cursor(term_column, term_row);
}

void terminal_cursor_right() {
    if (term_column < term_width - 1)
        term_column++;
    vga_update_cursor(term_column, term_row);
}

uint16_t terminal_get_row() {
    return term_row;
}

uint16_t terminal_get_column() {
    return term_column;
}
