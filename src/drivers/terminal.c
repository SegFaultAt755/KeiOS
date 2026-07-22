/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "drivers/terminal.h"

#include "kernel/qemu.h"
#include "libkern/string.h"

static uint16_t term_row;
static uint16_t term_column;
static uint8_t term_color;
static uint16_t *term_mem;

static uint16_t term_width;
static uint16_t term_height;

void terminal_initialize(uint16_t *mem, uint16_t width, uint16_t height) {
    term_mem = mem;
    term_width = width;
    term_height = height;
    term_color = vga_entry_color(TERMINAL_DEFAULT_FG, TERMINAL_DEFAULT_BG);

    terminal_clear();

    qemu_printf(QEMU_DRV, QEMU_OK, "Terminal initialized (memory: 0x%x, width: %u, height: %u, color: 0x%x)", term_mem,
                (uint32_t)term_width, (uint32_t)term_height, term_color);
}

void terminal_clear(void) {
    term_row = 0;
    term_column = 0;
    for (uint16_t y = 0; y < term_height; y++)
        for (uint16_t x = 0; x < term_width; x++)
            terminal_put_entry(' ', term_color, x, y);
}

void terminal_set_color(uint8_t color) {
    term_color = color;
}

void terminal_put_entry(char c, uint8_t color, uint16_t x, uint16_t y) {
    if (x < term_width && y < term_height) {
        const uint32_t i = y * term_width + x;
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
        return;
    }

    terminal_put_entry(c, term_color, term_column, term_row);

    if (++term_column >= term_width) {
        term_column = 0;
        if (++term_row >= term_height)
            terminal_scroll();
    }
}

void terminal_write(const char *s, size_t size) {
    for (size_t i = 0; i < size; i++)
        terminal_putchar(s[i]);
}

void terminal_writestring(const char *s) {
    for (size_t i = 0; s[i] != '\0'; i++)
        terminal_putchar(s[i]);
}

void terminal_blankline(void) {
    term_column = 0;
    if (++term_row >= term_height)
        terminal_scroll();
}

void terminal_scroll(void) {
    for (uint16_t y = 0; y < (term_height - 1); y++)
        for (uint16_t x = 0; x < term_width; x++)
            term_mem[y * term_width + x] = term_mem[(y + 1) * term_width + x];

    uint16_t l_row = term_height - 1;
    for (uint16_t x = 0; x < term_width; x++)
        term_mem[l_row * term_width + x] = vga_entry(' ', term_color);

    term_row = l_row;
}

void terminal_cursor_up(void) {
    if (term_row > 0)
        term_row--;
}

void terminal_cursor_down(void) {
    if (term_row < term_height - 1)
        term_row++;
}

void terminal_cursor_left(void) {
    if (term_column > 0)
        term_column--;
}

void terminal_cursor_right(void) {
    if (term_column < term_width - 1)
        term_column++;
}

uint16_t terminal_get_row(void) {
    return term_row;
}

uint16_t terminal_get_column(void) {
    return term_column;
}
