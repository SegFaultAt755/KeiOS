/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "libkern/memory.h"

void *memset(void *dest, int val, size_t n) {
    unsigned char *ptr = (unsigned char *)dest;

    while (n--)
        *ptr++ = val;

    return dest;
}

void *memcpy(void *dest, const void *src, size_t n) {
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;

    while (n--)
        *d++ = *s++;

    return dest;
}

void *memmove(void *dest, const void *src, size_t n) {
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;

    if (d == s)
        return dest;

    /* Check for overlap */
    if (d > s && d < s + n) {
        d += n;
        s += n;
        while (n--)
            *(--d) = *(--s);
    } else {
        while (n--)
            *d++ = *s++;
    }

    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const unsigned char *p1 = (const unsigned char *)s1;
    const unsigned char *p2 = (const unsigned char *)s2;

    while (n--) {
        if (*p1 != *p2)
            return *p1 - *p2;

        p1++;
        p2++;
    }

    return 0;
}
