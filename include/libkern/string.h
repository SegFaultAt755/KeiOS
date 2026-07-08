#pragma once

#include <stdint.h>

uint32_t strlen(const char *str);
int uvalue_to_str(char *buffer, unsigned int value, int base, int min_width, int zero_padding);
