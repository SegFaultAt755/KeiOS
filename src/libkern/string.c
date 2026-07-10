/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "libkern/string.h"

uint32_t strlen(const char *s) {
    uint32_t i = 0;
    for (; s[i] != '\0'; i++) {
    }

    return i;
}

int uvalue_to_str(char *buf, unsigned int val, int base, int min_w, int zpad) {
    int i = 0;

    if (val == 0) {
        buf[i++] = '0';
    } else {
        while (val > 0) {
            int rem = val % base;
            buf[i++] = (rem < 10) ? (rem + '0') : (rem - 10 + 'a');
            val /= base;
        }
    }

    /* Apply zero padding */
    while (i < min_w && i < 32 && zpad)
        buf[i++] = '0';

    /* Reverse the buffer */
    for (int j = 0; j < i / 2; j++) {
        char tmp = buf[j];
        buf[j] = buf[i - 1 - j];
        buf[i - 1 - j] = tmp;
    }

    return i; /* Returns length of the string */
}
