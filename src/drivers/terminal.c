#include "drivers/terminal.h"
#include "libkern/string.h"

uint16_t terminal_row;
uint16_t terminal_column;
uint8_t  terminal_color;
uint16_t* terminal_buffer = (uint16_t*) VGA_MEMORY;

void terminal_initialize(void) {
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = vga_entry_color(TERMINAL_DEFAULT_FOREGROUND_COLOR, TERMINAL_DEFAULT_BACKGROUND_COLOR);

    for (uint16_t y = 0; y < VGA_HEIGHT; y++) {
        for (uint16_t x = 0; x < VGA_WIDTH; x++) {
            const uint32_t index = y * VGA_WIDTH + x;
            terminal_buffer[index] = vga_entry(' ', terminal_color);
        }
    }
}

void terminal_set_color(uint8_t color) {
    terminal_color = color;
}

void terminal_put_entry(char c, uint8_t color, uint16_t x, uint16_t y) {
    const uint32_t index = y * VGA_WIDTH + x;
    terminal_buffer[index] = vga_entry(c, color);
}

void terminal_put_char(char c) {
    if (c == '\n') {
        terminal_blankline();
        return;
    }

    terminal_put_entry(c, terminal_color, terminal_column, terminal_row);

    if (++terminal_column >= VGA_WIDTH) {
        terminal_column = 0;
        if (++terminal_row >= VGA_HEIGHT)
            terminal_row = 0;
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

void terminal_blankline() {
    terminal_column = 0;
    terminal_row++;
    if (terminal_row >= VGA_HEIGHT)
        terminal_scroll();
}

void terminal_scroll() {
    for(uint16_t i = 0; i < VGA_HEIGHT; i++){
        for (uint16_t m = 0; m < VGA_WIDTH; m++){
            terminal_buffer[i * VGA_WIDTH + m] = terminal_buffer[(i + 1) * VGA_WIDTH + m];
        }
    }
}