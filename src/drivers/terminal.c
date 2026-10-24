/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "drivers/terminal.h"

#include "drivers/display.h"
#include "kernel/qemu.h"
#include "libkern/string.h"

constexpr uint32_t TERMINAL_FONT_WIDTH = 16;
constexpr uint32_t TERMINAL_FONT_HEIGHT = 16;

static const uint32_t terminal_palette[] = {
    0x000000, 0x0000AA, 0x00AA00, 0x00AAAA, 0xAA0000, 0xAA00AA, 0xAA5500, 0xAAAAAA,
    0x555555, 0x5555FF, 0x55FF55, 0x55FFFF, 0xFF5555, 0xFF55FF, 0xFFFF55, 0xFFFFFF,
};

static uint16_t term_row;
static uint16_t term_column;
static uint8_t term_color;
static uint16_t *term_mem = nullptr;
static bool term_use_display;

static uint16_t term_width;
static uint16_t term_height;

void terminal_init(uint16_t *mem, uint16_t width, uint16_t height) {
    term_use_display = mem == nullptr && display_is_initialized();
    term_mem = term_use_display ? nullptr : mem;
    term_width = width;
    term_height = height;
    term_color = vga_entry_color(TERMINAL_DEFAULT_FG, TERMINAL_DEFAULT_BG);

    terminal_clear();
    if (!term_use_display) {
        vga_enable_cursor(14, 15);
        vga_update_cursor(term_column, term_row);
    }

    qemu_printf(QEMU_DRV, QEMU_OK, "Terminal initialized (memory: %p, width: %u, height: %u, color: %p)", term_mem,
                (uint32_t)term_width, (uint32_t)term_height, term_color);
}

void terminal_clear() {
    term_row = 0;
    term_column = 0;
    if (term_use_display) {
        auto bg = terminal_palette[(term_color >> 4) & 0x0F];
        display_clear(bg);
    } else {
        for (auto y = 0u; y < term_height; y++)
            for (auto x = 0u; x < term_width; x++)
                terminal_put_entry(' ', term_color, x, y);
        vga_update_cursor(term_column, term_row);
    }
}

void terminal_set_color(uint8_t color) {
    term_color = color;
}

void terminal_put_entry(char c, uint8_t color, uint16_t x, uint16_t y) {
    if (x < term_width && y < term_height) {
        if (term_use_display) {
            auto px = (int32_t)x * (int32_t)TERMINAL_FONT_WIDTH;
            auto py = (int32_t)y * (int32_t)TERMINAL_FONT_HEIGHT;
            auto bg = terminal_palette[(color >> 4) & 0x0F];
            auto fg = terminal_palette[color & 0x0F];
            display_fill_rect(px, py, TERMINAL_FONT_WIDTH, TERMINAL_FONT_HEIGHT, bg);
            if (c != ' ') {
                auto character = (unsigned char)c;
                char text[] = {character >= ' ' && character <= '~' ? c : '?', '\0'};
                display_draw_text_static(px, py, text, fg);
            }
        } else {
            auto i = (uint32_t)y * term_width + x;
            term_mem[i] = vga_entry(c, color);
        }
    }
}

static void terminal_update_cursor() {
    if (!term_use_display)
        vga_update_cursor(term_column, term_row);
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

        terminal_update_cursor();
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
        terminal_update_cursor();
        return;
    }

    terminal_put_entry(c, term_color, term_column, term_row);

    if (++term_column >= term_width) {
        term_column = 0;
        if (++term_row >= term_height)
            terminal_scroll();
    }

    terminal_update_cursor();
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
    terminal_update_cursor();
}

void terminal_scroll() {
    if (term_height == 0)
        return;

    auto bg = terminal_palette[(term_color >> 4) & 0x0F];
    if (term_use_display) {
        display_scroll_up(TERMINAL_FONT_HEIGHT, bg);
    } else {
        for (auto y = 0u; y < (uint16_t)(term_height - 1); y++)
            for (auto x = 0u; x < term_width; x++)
                term_mem[y * term_width + x] = term_mem[(y + 1) * term_width + x];

        auto l_row = term_height - 1;
        for (auto x = 0u; x < term_width; x++)
            term_mem[l_row * term_width + x] = vga_entry(' ', term_color);
    }

    term_row = term_height - 1;
    terminal_update_cursor();
}

void terminal_cursor_up() {
    if (term_row > 0)
        term_row--;
    terminal_update_cursor();
}

void terminal_cursor_down() {
    if (term_row < term_height - 1)
        term_row++;
    terminal_update_cursor();
}

void terminal_cursor_left() {
    if (term_column > 0)
        term_column--;
    terminal_update_cursor();
}

void terminal_cursor_right() {
    if (term_column < term_width - 1)
        term_column++;
    terminal_update_cursor();
}

uint16_t terminal_get_row() {
    return term_row;
}

uint16_t terminal_get_column() {
    return term_column;
}
