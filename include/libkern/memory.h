#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <stddef.h>

void *memset(void *dest, int value, size_t n);
void *memcpy(void *dest, const void *src, size_t n);

#endif