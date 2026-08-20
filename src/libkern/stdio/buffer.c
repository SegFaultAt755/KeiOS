/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "drivers/terminal.h"
#include "libkern/stdio.h"
#include "libkern/string.h"

#define EMIT_CHAR(c)                                                                                                   \
    do {                                                                                                               \
        if (buf != NULL && size > 0 && buf_idx < size - 1) {                                                           \
            buf[buf_idx] = (c);                                                                                        \
        }                                                                                                              \
        buf_idx++;                                                                                                     \
    } while (0)

static constexpr size_t buf_num_len = 70;

int kvsnprintf(char *buf, size_t size, const char *fmt, va_list args) {
    if (fmt == nullptr)
        return 0;

    size_t buf_idx = 0;
    for (size_t i = 0; fmt[i] != '\0'; i++) {
        if (fmt[i] == '%' && fmt[i + 1] != '\0') {
            i++;

            bool zpad = false;
            int min_w = 0;

            /* Check whether zero padding was requested */
            if (fmt[i] == '0') {
                zpad = true;
                i++;
            }

            /* Parse the minimum width separately from zero padding */
            while (fmt[i] >= '0' && fmt[i] <= '9') {
                /* Prevent integer overflow while parsing the width */
                if (min_w < 10000) {
                    min_w = min_w * 10 + (fmt[i] - '0');
                }

                i++;
            }

            char buf_num[buf_num_len] = {};

            if (min_w > (int)sizeof(buf_num) - 2) {
                min_w = (int)sizeof(buf_num) - 2;
            }

            /* Parse the length modifier. */
            enum {
                LEN_DEFAULT,
                LEN_LONG
            } len_mod = LEN_DEFAULT;
            if (fmt[i] == 'l') {
                len_mod = LEN_LONG;
                i++;
            }

            int len = 0;

            switch (fmt[i]) {
            case 'd':
            case 'i': {
                long long int val = 0;
                if (len_mod == LEN_LONG)
                    val = va_arg(args, long long int);
                else
                    val = va_arg(args, int);

                unsigned long long int uval;
                if (val < 0) {
                    EMIT_CHAR('-');
                    uval = ~((unsigned long long int)val) + 1;
                } else {
                    uval = (unsigned long long int)val;
                }

                len = uvalue_to_str(buf_num, uval, 10, min_w, zpad);
                break;
            }
            case 'u':
            case 'x':
            case 'o':
            case 'b': {
                unsigned long long int uval = 0;
                if (len_mod == LEN_LONG)
                    uval = va_arg(args, unsigned long long int);
                else
                    uval = va_arg(args, unsigned int);

                auto base = 10;
                if (fmt[i] == 'x')
                    base = 16;
                else if (fmt[i] == 'o')
                    base = 8;
                else if (fmt[i] == 'b')
                    base = 2;

                len = uvalue_to_str(buf_num, uval, base, min_w, zpad);
                break;
            }
            case 'p': {
                auto ptr = va_arg(args, void *);
                EMIT_CHAR('0');
                EMIT_CHAR('x');

                len = uvalue_to_str(buf_num, (unsigned long long)(uintptr_t)ptr, 16, sizeof(void *) * 2, true);
                break;
            }
            case 'f': {
                auto val = va_arg(args, double);

                /* Check for a not-a-number value */
                if (val != val) {
                    constexpr char nan_str[] = "NaN";
                    for (int k = 0; nan_str[k] != '\0'; k++)
                        EMIT_CHAR(nan_str[k]);
                    break;
                }

                if (val < 0) {
                    EMIT_CHAR('-');
                    val = -val;
                }

                /* Adjust rounding for a precision of 6 decimal places */
                val += 0.0000005;
                auto ipart = (unsigned long long int)val;
                auto fpart = val - (double)ipart;
                auto fpart_int = (unsigned long long int)(fpart * 1000000.0);

                /* Format and copy the integer part */
                len = uvalue_to_str(buf_num, ipart, 10, min_w, zpad);
                for (int k = 0; k < len; k++)
                    EMIT_CHAR(buf_num[k]);

                /* Add the decimal point */
                EMIT_CHAR('.');

                /* Format the fractional part */
                len = uvalue_to_str(buf_num, fpart_int, 10, 6, true);
                break;
            }
            case 'c':
                EMIT_CHAR((char)va_arg(args, int));
                continue;
            case 's': {
                auto s = va_arg(args, const char *);
                if (!s)
                    s = "(null)";
                while (*s)
                    EMIT_CHAR(*s++);
                continue;
            }
            case '%':
                EMIT_CHAR('%');
                continue;
            default:
                EMIT_CHAR('%');
                EMIT_CHAR(fmt[i]);
                continue;
            }

            /* Copy the formatted number buffers without overrunning them */
            for (int k = 0; k < len; k++)
                EMIT_CHAR(buf_num[k]);
        } else {
            EMIT_CHAR(fmt[i]);
        }
    }

    /* Ensure that the string ends with a null character */
    if (buf != NULL && size > 0) {
        size_t term_idx = (buf_idx < size) ? buf_idx : (size - 1);
        buf[term_idx] = '\0';
    }

    return (int)buf_idx;
}

int ksnprintf(char *buf, size_t size, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    auto res = kvsnprintf(buf, size, fmt, args);
    va_end(args);

    return res;
}
