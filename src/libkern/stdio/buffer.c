/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "drivers/terminal.h"
#include "libkern/stdio.h"
#include "libkern/string.h"

int kvsnprintf(char *buf, size_t size, const char *fmt, va_list args) {
    if (size == 0)
        return 0;

    size_t buf_idx = 0;
    for (size_t i = 0; fmt[i] != '\0' && buf_idx < size - 1; i++) {
        if (fmt[i] == '%' && fmt[i + 1] != '\0') {
            i++;

            int zpad = 0;
            int min_w = 0;

            if (fmt[i] == '0') {
                zpad = 1;
                i++;

                while (fmt[i] >= '0' && fmt[i] <= '9') {
                    min_w = min_w * 10 + (fmt[i] - '0');
                    i++;
                }
            }

            char buf_num[32];
            int len = 0;

            switch (fmt[i]) {
            case 'd':
            case 'i': {
                int val = va_arg(args, int);
                unsigned int uval = (unsigned int)val;
                if (val < 0 && buf_idx < size - 1) {
                    buf[buf_idx++] = '-';
                    uval = ~uval + 1;
                }

                len = uvalue_to_str(buf_num, uval, 10, min_w, zpad);
                break;
            }
            case 'u':
                len = uvalue_to_str(buf_num, va_arg(args, unsigned int), 10, min_w, zpad);
                break;
            case 'x':
                len = uvalue_to_str(buf_num, va_arg(args, unsigned int), 16, min_w, zpad);
                break;
            case 'o':
                len = uvalue_to_str(buf_num, va_arg(args, unsigned int), 8, min_w, zpad);
                break;
            case 'b':
                len = uvalue_to_str(buf_num, va_arg(args, unsigned int), 2, min_w, zpad);
                break;
            case 'p': {
                void *ptr = va_arg(args, void *);
                if (buf_idx < size - 1)
                    buf[buf_idx++] = '0';
                if (buf_idx < size - 1)
                    buf[buf_idx++] = 'x';

                len = uvalue_to_str(buf_num, (unsigned long long)(uintptr_t)ptr, 16, sizeof(void *) * 2, 1);
                break;
            }
            case 'f': {
                double val = va_arg(args, double);

                /* Check for NaN */
                if (val != val) {
                    const char *nan_str = "NaN";
                    for (int k = 0; nan_str[k] != '\0' && buf_idx < size - 1; k++)
                        buf[buf_idx++] = nan_str[k];
                    break;
                }

                if (val < 0 && buf_idx < size - 1) {
                    buf[buf_idx++] = '-';
                    val = -val;
                }

                /* Rounding adjustment for 6 decimal places precision */
                val += 0.0000005f;
                unsigned int ipart = (unsigned int)val;
                double fpart = val - (double)ipart;
                unsigned int fpart_int = (unsigned int)(fpart * 1000000.0f);

                /* Format and copy the integer part */
                len = uvalue_to_str(buf_num, ipart, 10, min_w, zpad);
                for (int k = 0; k < len && buf_idx < size - 1; k++)
                    buf[buf_idx++] = buf_num[k];

                /* Place the decimal point */
                if (buf_idx < size - 1)
                    buf[buf_idx++] = '.';

                /* Format the fractional part */
                len = uvalue_to_str(buf_num, fpart_int, 10, 6, 1);
                break;
            }
            case 'c':
                if (buf_idx < size - 1)
                    buf[buf_idx++] = (char)va_arg(args, int);
                continue;
            case 's': {
                const char *s = va_arg(args, const char *);
                if (!s)
                    s = "(null)";
                while (*s && buf_idx < size - 1)
                    buf[buf_idx++] = *s++;
                continue;
            }
            case '%':
                if (buf_idx < size - 1)
                    buf[buf_idx++] = '%';
                continue;
            default:
                if (buf_idx < size - 1)
                    buf[buf_idx++] = '%';
                if (buf_idx < size - 1)
                    buf[buf_idx++] = fmt[i];
                continue;
            }

            /* Copy formatted numerical buffers over */
            for (int k = 0; k < len && buf_idx < size - 1; k++)
                buf[buf_idx++] = buf_num[k];
        } else {
            buf[buf_idx++] = fmt[i];
        }
    }

    buf[buf_idx] = '\0'; /* Ensure null-termination */
    return (int)buf_idx;
}

int ksnprintf(char *buf, size_t size, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int res = kvsnprintf(buf, size, fmt, args);
    va_end(args);

    return res;
}
