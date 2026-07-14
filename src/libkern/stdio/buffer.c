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

                len = uvalue_to_str(buf_num, uval, 10, 0, 0);
                break;
            }
            case 'u':
                len = uvalue_to_str(buf_num, va_arg(args, unsigned int), 10, 0, 0);
                break;
            case 'x':
                len = uvalue_to_str(buf_num, va_arg(args, unsigned int), 16, 0, 0);
                break;
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
