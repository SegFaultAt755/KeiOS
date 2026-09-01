/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "libkern/string.h"
#include "arch/x86/heap.h"
#include "kernel/panic.h"

size_t strlen(const char *s) {
    KERNEL_ASSERT(s != nullptr, "Invalid string pointer",
                  "strlen called with NULL pointer - cannot determine length of null string");

    size_t i = 0;
    for (; s[i] != '\0'; i++)
        ;

    return i;
}

char *strcpy(char *dest, const char *src) {
    KERNEL_ASSERT(dest != nullptr, "Invalid destination buffer", "strcpy called with NULL destination pointer");
    KERNEL_ASSERT(src != nullptr, "Invalid source string", "strcpy called with NULL source pointer");

    auto tmp = dest;

    while ((*dest++ = *src++) != '\0')
        ;

    return tmp;
}

int strcmp(const char *s1, const char *s2) {
    KERNEL_ASSERT(s1 != nullptr, "Invalid string pointer", "strcmp called with NULL first string pointer");
    KERNEL_ASSERT(s2 != nullptr, "Invalid string pointer", "strcmp called with NULL second string pointer");

    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }

    return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
    KERNEL_ASSERT(s1 != nullptr, "Invalid string pointer", "strncmp called with NULL first string pointer");
    KERNEL_ASSERT(s2 != nullptr, "Invalid string pointer", "strncmp called with NULL second string pointer");

    for (size_t i = 0; i < n; i++) {
        if (s1[i] != s2[i])
            return *(const unsigned char *)&s1[i] - *(const unsigned char *)&s2[i];
        if (s1[i] == '\0')
            return 0;
    }

    return 0;
}

char *strcat(char *dest, const char *src) {
    KERNEL_ASSERT(dest != nullptr, "Invalid destination buffer", "strcat called with NULL destination pointer");
    KERNEL_ASSERT(src != nullptr, "Invalid source string", "strcat called with NULL source pointer");

    auto tmp = dest;

    while (*dest)
        dest++;

    while ((*dest++ = *src++) != '\0')
        ;

    return tmp;
}

char *strchr(const char *s, int c) {
    KERNEL_ASSERT(s != nullptr, "Invalid string pointer",
                  "strchr called with NULL pointer - cannot search in null string");

    while (*s) {
        if (*s == (char)c)
            return (char *)s;
        s++;
    }

    return (c == '\0') ? (char *)s : nullptr;
}

char *strtok(char *str, const char *delim) {
    KERNEL_ASSERT(delim != nullptr, "Invalid delimiter string", "strtok called with NULL delimiter pointer");

    static char *next = nullptr;

    if (str)
        next = str;

    if (!next)
        return nullptr;

    /* Skip delimiters at the start of the string */
    while (*next) {
        auto d = delim;
        bool is_delim = false;
        while (*d) {
            if (*next == *d) {
                is_delim = true;
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

    auto token = next;

    /* Find the end of the current token */
    while (*next) {
        auto d = delim;
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
    KERNEL_ASSERT(s != nullptr, "Invalid string pointer",
                  "strdup called with NULL pointer - cannot duplicate null string");

    auto len = strlen(s) + 1;
    auto dup = (char *)kmalloc(len);

    KERNEL_ASSERT(dup != nullptr, "Memory allocation failed during strdup",
                  "kmalloc failed to allocate memory for string duplication");

    strcpy(dup, s);
    return dup;
}

int atoi(const char *s) {
    KERNEL_ASSERT(s != nullptr, "Invalid string pointer",
                  "atoi called with NULL pointer - cannot parse integer from null string");

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
    KERNEL_ASSERT(buf != nullptr, "Invalid buffer pointer",
                  "itoa called with NULL buffer pointer - cannot write integer to null buffer");
    KERNEL_ASSERT(base >= 2 && base <= 36, "Invalid base for itoa",
                  "itoa called with invalid base - base must be between 2 and 36");

    int i = 0;
    bool is_negative = false;

    if (val == 0) {
        buf[i++] = '0';
        buf[i] = '\0';
        return;
    }

    if (val < 0 && base == 10) {
        is_negative = true;
        val = -val;
    }

    while (val > 0) {
        auto rem = val % base;
        buf[i++] = (rem < 10) ? (rem + '0') : (rem - 10 + 'a');
        val /= base;
    }

    if (is_negative)
        buf[i++] = '-';

    buf[i] = '\0';

    /* Reverse the string */
    for (int j = 0; j < i / 2; j++) {
        char tmp = buf[j];
        buf[j] = buf[i - 1 - j];
        buf[i - 1 - j] = tmp;
    }
}

int uvalue_to_str(char *buf, unsigned long long int val, int base, int min_w, bool zpad) {
    KERNEL_ASSERT(buf != nullptr, "Invalid buffer pointer",
                  "uvalue_to_str called with NULL buffer pointer - cannot write to null buffer");
    KERNEL_ASSERT(base >= 2 && base <= 36, "Invalid base for uvalue_to_str",
                  "uvalue_to_str called with invalid base - base must be between 2 and 36");
    KERNEL_ASSERT(min_w >= 0 && min_w <= 32, "Invalid minimum width",
                  "uvalue_to_str called with invalid min_w - minimum width must be between 0 and 32");

    int i = 0;

    if (val == 0) {
        buf[i++] = '0';
    } else {
        while (val > 0) {
            auto rem = val % base;
            buf[i++] = (rem < 10) ? (rem + '0') : (rem - 10 + 'a');
            val /= base;
        }
    }

    /* Add leading zeroes when padding is requested */
    while (i < min_w && i < 32 && zpad)
        buf[i++] = '0';

    /* Reverse the output buffer */
    for (int j = 0; j < i / 2; j++) {
        char tmp = buf[j];
        buf[j] = buf[i - 1 - j];
        buf[i - 1 - j] = tmp;
    }

    return i; /* Return the string length */
}
