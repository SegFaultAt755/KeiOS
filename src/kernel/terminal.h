#ifndef __TERMINAL_H__
#define __TERMINAL_H__

#include "stdint.h"
#include "vga.h"

void terminal_initialize(void);
void terminal_set_color(uint8_t color);
void terminal_put_entry(char c, uint8_t color, uint16_t x, uint16_t y);
void terminal_put_char(char c);
void terminal_write(const char *str, uint16_t size);

#endif