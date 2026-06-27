#include "libkern/memory.h"

void *memset(void *dest, int value, size_t n) {
    unsigned char *ptr = (unsigned char *) dest;
    unsigned char val = (unsigned char) value;
    
    while (n--) {*ptr++ = val;}
    return dest;
}

void *memcpy(void *dest, const void *src, size_t n) {
    unsigned char *d = (unsigned char *) dest;
    const unsigned char *s = (const unsigned char *) src;
    
    while (n--) {*d++ = *s++;}
    return dest;
}
