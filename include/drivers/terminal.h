#pragma once

#include <stdint.h>
#include <stddef.h>
#include "drivers/vga.h"

#define TERMINAL_DEFAULT_FG VGA_COLOR_WHITE
#define TERMINAL_DEFAULT_BG VGA_COLOR_BLACK
#define TERMINAL_TAB_SIZE 4

void terminal_initialize(uint16_t *mem, uint16_t width, uint16_t height);
void terminal_clear(void);

void terminal_set_color(uint8_t color);

void terminal_put_entry(char c, uint8_t color, uint16_t x, uint16_t y);
void terminal_putchar(char c);

void terminal_write(const char *s, size_t len);
void terminal_writestring(const char *s);

void terminal_blankline(void);
void terminal_scroll(void);
