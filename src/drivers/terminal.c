#include "drivers/terminal.h"
#include "libkern/string.h"
#include "kernel/qemu.h"

static uint16_t  terminal_row;
static uint16_t  terminal_column;
static uint8_t   terminal_color;
static uint16_t *terminal_buffer;

static uint16_t terminal_width;
static uint16_t terminal_height;

void terminal_initialize(uint16_t *buffer, uint16_t width, uint16_t height) {
    qemu_printf(QEMU_LOG_INFO, "Initializating terminal : {%x, %d, %d}", buffer, width, height);

    terminal_buffer = buffer;
    terminal_width  = width;
    terminal_height = height;
    terminal_color  = vga_entry_color(TERMINAL_DEFAULT_FOREGROUND_COLOR, TERMINAL_DEFAULT_BACKGROUND_COLOR);
    
    terminal_clear();
}

void terminal_clear(void) {
    terminal_row = 0;
    terminal_column = 0;
    for (uint16_t y = 0; y < terminal_height; y++) {
        for (uint16_t x = 0; x < terminal_width; x++) {
            terminal_put_entry(' ', terminal_color, x, y);
        }
    }
}

void terminal_set_color(uint8_t color) {
    terminal_color = color;
}

void terminal_put_entry(char c, uint8_t color, uint16_t x, uint16_t y) {
    if (x < terminal_width && y < terminal_height) {
        const uint32_t index = y * terminal_width + x;
        terminal_buffer[index] = vga_entry(c, color);
    }
}

void terminal_put_char(char c) {
    if (c == '\n') {
        terminal_blankline();
        return;
    }

    if (c == '\t') {
        terminal_column += TERMINAL_TAB_SIZE - (terminal_column % TERMINAL_TAB_SIZE);
        if (terminal_column >= terminal_width) {
            terminal_column = 0;
            if (++terminal_row >= terminal_height)
                terminal_scroll();
        }

        return;
    }

    if (c == '\b') {
        if (terminal_column > 0) {
            terminal_column--;
        } else if (terminal_row > 0) {
            terminal_row--;
            terminal_column = terminal_width - 1;
        }

        terminal_put_entry(' ', terminal_color, terminal_column, terminal_row);
        return;
    }

    terminal_put_entry(c, terminal_color, terminal_column, terminal_row);

    if (++terminal_column >= terminal_width) {
        terminal_column = 0;
        if (++terminal_row >= terminal_height)
            terminal_scroll();
    }
}

void terminal_write(const char *str, size_t size) {
    for (size_t i = 0; i < size; i++)
        terminal_put_char(str[i]);
}

void terminal_writestring(const char *str) {
    for (size_t i = 0; str[i] != '\0'; i++)
        terminal_put_char(str[i]);
}

void terminal_blankline(void) {
    terminal_column = 0;
    if (++terminal_row >= terminal_height)
        terminal_scroll();
}

void terminal_scroll(void) {
    for (uint16_t y = 0; y < (terminal_height - 1); y++)
        for (uint16_t x = 0; x < terminal_width; x++)
            terminal_buffer[y * terminal_width + x] = terminal_buffer[(y + 1) * terminal_width + x];

    uint16_t last_row = terminal_height - 1;
    for (uint16_t x = 0; x < terminal_width; x++)
        terminal_buffer[last_row * terminal_width + x] = vga_entry(' ', terminal_color);

    terminal_row = last_row;
}
