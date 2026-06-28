#ifndef __TERMINAL_H__
#define __TERMINAL_H__

#include <stdint.h>
#include <stddef.h>
#include "drivers/vga.h"

#define TERMINAL_DEFAULT_FOREGROUND_COLOR VGA_COLOR_WHITE
#define TERMINAL_DEFAULT_BACKGROUND_COLOR VGA_COLOR_BLACK

void terminal_initialize(void);
void terminal_set_color(uint8_t color);
void terminal_put_entry(char c, uint8_t color, uint16_t x, uint16_t y);
void terminal_put_char(char c);
void terminal_write(const char *str, size_t size);
void terminal_writestring(const char *str);
void terminal_blankline(void);
void terminal_scroll(void);

#endif