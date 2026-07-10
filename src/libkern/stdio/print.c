/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "drivers/terminal.h"
#include "libkern/stdio.h"
#include "libkern/string.h"

void kvprintf(const char *fmt, va_list args) {
    for (size_t i = 0; fmt[i] != '\0'; i++) {
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

            char buf[32];
            int len = 0;

            switch (fmt[i]) {
            case 'd':
            case 'i': {
                int val = va_arg(args, int);
                unsigned int uval = (unsigned int)val;
                if (val < 0) {
                    terminal_putchar('-');
                    uval = ~uval + 1;
                }

                len = uvalue_to_str(buf, uval, 10, min_w, zpad);
                for (int k = 0; k < len; k++)
                    terminal_write(&buf[k], 1);
                break;
            }
            case 'u':
                len = uvalue_to_str(buf, va_arg(args, unsigned int), 10, min_w, zpad);
                for (int k = 0; k < len; k++)
                    terminal_write(&buf[k], 1);
                break;
            case 'x':
                len = uvalue_to_str(buf, va_arg(args, unsigned int), 16, min_w, zpad);
                for (int k = 0; k < len; k++)
                    terminal_write(&buf[k], 1);
                break;
            case 'o':
                len = uvalue_to_str(buf, va_arg(args, unsigned int), 8, min_w, zpad);
                for (int k = 0; k < len; k++)
                    terminal_write(&buf[k], 1);
                break;
            case 'b':
                len = uvalue_to_str(buf, va_arg(args, unsigned int), 2, min_w, zpad);
                for (int k = 0; k < len; k++)
                    terminal_write(&buf[k], 1);
                break;
            case 'p': {
                void *ptr = va_arg(args, void *);
                terminal_writestring("0x");

                len = uvalue_to_str(buf, (unsigned long long)(uintptr_t)ptr, 16, sizeof(void *) * 2, 1);
                for (int k = 0; k < len; k++)
                    terminal_write(&buf[k], 1);
                break;
            }
            case 'c': {
                char c = (char)va_arg(args, int);
                terminal_write(&c, 1);
                break;
            }
            case 's': {
                const char *s = va_arg(args, const char *);
                terminal_writestring(s ? s : "(null)");
                break;
            }
            case '%':
                terminal_putchar('%');
                break;
            default:
                terminal_putchar('%');
                terminal_write(&fmt[i], 1);
                break;
            }
        } else {
            terminal_write(&fmt[i], 1);
        }
    }
}

void kprintf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    kvprintf(fmt, args);
    va_end(args);
}
