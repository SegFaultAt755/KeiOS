/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "libkern/string.h"

#include "arch/x86/heap.h"

size_t strlen(const char *s) {
    size_t i = 0;
    for (; s[i] != '\0'; i++) {
    }

    return i;
}

char *strcpy(char *dest, const char *src) {
    char *tmp = dest;

    while ((*dest++ = *src++) != '\0') {
    }

    return tmp;
}

int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }

    return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
    for (size_t i = 0; i < n; i++) {
        if (s1[i] != s2[i])
            return *(const unsigned char *)&s1[i] - *(const unsigned char *)&s2[i];
        if (s1[i] == '\0')
            return 0;
    }

    return 0;
}

char *strcat(char *dest, const char *src) {
    char *tmp = dest;

    while (*dest)
        dest++;

    while ((*dest++ = *src++) != '\0') {
    }

    return tmp;
}

char *strchr(const char *s, int c) {
    while (*s) {
        if (*s == (char)c)
            return (char *)s;
        s++;
    }

    return (c == '\0') ? (char *)s : nullptr;
}

char *strtok(char *str, const char *delim) {
    static char *next = nullptr;

    if (str)
        next = str;

    if (!next)
        return nullptr;

    /* Skip leading delimiters */
    while (*next) {
        const char *d = delim;
        int is_delim = 0;
        while (*d) {
            if (*next == *d) {
                is_delim = 1;
                break;
            }
            d++;
        }
        if (!is_delim)
            break;
        next++;
    }

    if (*next == '\0')
        return nullptr;

    char *token = next;

    /* Find end of token */
    while (*next) {
        const char *d = delim;
        while (*d) {
            if (*next == *d) {
                *next = '\0';
                next++;
                return token;
            }
            d++;
        }
        next++;
    }

    return token;
}

char *strdup(const char *s) {
    size_t len = strlen(s) + 1;
    char *dup = kmalloc(len);
    if (dup)
        strcpy(dup, s);
    return dup;
}

int atoi(const char *s) {
    int result = 0;
    int sign = 1;

    while (*s == ' ')
        s++;

    if (*s == '-') {
        sign = -1;
        s++;
    } else if (*s == '+') {
        s++;
    }

    while (*s >= '0' && *s <= '9') {
        result = result * 10 + (*s - '0');
        s++;
    }

    return result * sign;
}

void itoa(int val, char *buf, int base) {
    int i = 0;
    int is_negative = 0;

    if (val == 0) {
        buf[i++] = '0';
        buf[i] = '\0';
        return;
    }

    if (val < 0 && base == 10) {
        is_negative = 1;
        val = -val;
    }

    while (val > 0) {
        int rem = val % base;
        buf[i++] = (rem < 10) ? (rem + '0') : (rem - 10 + 'a');
        val /= base;
    }

    if (is_negative)
        buf[i++] = '-';

    buf[i] = '\0';

    /* Reverse */
    for (int j = 0; j < i / 2; j++) {
        char tmp = buf[j];
        buf[j] = buf[i - 1 - j];
        buf[i - 1 - j] = tmp;
    }
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
