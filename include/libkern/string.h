#pragma once

#include <stdint.h>

uint32_t strlen(const char *s);
int uvalue_to_str(char *buf, unsigned int val, int base, int min_w, int zpad);
