#include "libkern/stdio.h"
#include "drivers/terminal.h"
#include "libkern/string.h"

void kprint_uint(unsigned int val, int base) {
    char buf[32];
    int len = uvalue_to_str(buf, val, base, 0, 0);
    for (int i = 0; i < len; i++)
        terminal_write(&buf[i], 1);
}

void kprint_int(int val) {
    unsigned int uval = (unsigned int) val;
    if (val < 0) {
        terminal_writestring("-");
        uval = ~uval + 1;
    }

    kprint_uint(uval, 10);
}

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
                    unsigned int uval = (unsigned int) val;
                    if (val < 0) {
                        terminal_writestring("-");
                        uval = ~uval + 1;
                    }

                    len = uvalue_to_str(buf, uval, 10, min_w, zpad);
                    for (int k = 0; k < len; k++) terminal_write(&buf[k], 1);
                    break;
                }
                case 'u':
                    len = uvalue_to_str(buf, va_arg(args, unsigned int), 10, min_w, zpad);
                    for (int k = 0; k < len; k++) terminal_write(&buf[k], 1);
                    break;
                case 'x':
                    len = uvalue_to_str(buf, va_arg(args, unsigned int), 16, min_w, zpad);
                    for (int k = 0; k < len; k++) terminal_write(&buf[k], 1);
                    break;
                case 'o':
                    len = uvalue_to_str(buf, va_arg(args, unsigned int), 8, min_w, zpad);
                    for (int k = 0; k < len; k++) terminal_write(&buf[k], 1);
                    break;
                case 'b':
                    len = uvalue_to_str(buf, va_arg(args, unsigned int), 2, min_w, zpad);
                    for (int k = 0; k < len; k++) terminal_write(&buf[k], 1);
                    break;
                case 'p': {
                    void *ptr = va_arg(args, void *);
                    terminal_writestring("0x");

                    len = uvalue_to_str(buf, (unsigned long long)(uintptr_t)ptr, 16, sizeof(void*) * 2, 1);
                    for (int k = 0; k < len; k++) terminal_write(&buf[k], 1);
                    break;
                }
                case 'c': {
                    char c = (char) va_arg(args, int);
                    terminal_write(&c, 1);
                    break;
                }
                case 's': {
                    const char *s = va_arg(args, const char *);
                    terminal_writestring(s ? s : "(null)");
                    break;
                }
                case '%':
                    terminal_writestring("%");
                    break;
                default:
                    terminal_writestring("%");
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

/* Safe string formatting function */
int kvsnprintf(char *buf, size_t size, const char *fmt, va_list args) {
    if (size == 0) return 0;

    size_t buf_idx = 0;
    for (size_t i = 0; fmt[i] != '\0' && buf_idx < size - 1; i++) {
        if (fmt[i] == '%' && fmt[i + 1] != '\0') {
            i++;
            char buffer[32];
            int length = 0;

            switch (fmt[i]) {
                case 'd':
                case 'i': {
                    int value = va_arg(args, int);
                    unsigned int uvalue = (unsigned int) value;
                    if (value < 0 && buf_idx < size - 1) {
                        buf[buf_idx++] = '-';
                        uvalue = ~uvalue + 1;
                    }

                    length = uvalue_to_str(buffer, uvalue, 10, 0, 0);
                    break;
                }
                case 'u':
                    length = uvalue_to_str(buffer, va_arg(args, unsigned int), 10, 0, 0);
                    break;
                case 'x':
                    length = uvalue_to_str(buffer, va_arg(args, unsigned int), 16, 0, 0);
                    break;
                case 'c':
                    if (buf_idx < size - 1)
                        buf[buf_idx++] = (char) va_arg(args, int);
                    continue;
                case 's': {
                    const char *s = va_arg(args, const char *);
                    if (!s) s = "(null)";
                    while (*s && buf_idx < size - 1)
                        buf[buf_idx++] = *s++;
                    continue;
                }
                case '%':
                    if (buf_idx < size - 1) buf[buf_idx++] = '%';
                    continue;
                default:
                    if (buf_idx < size - 1) buf[buf_idx++] = '%';
                    if (buf_idx < size - 1) buf[buf_idx++] = fmt[i];
                    continue;
            }

            /* Copy formatted numerical buffers over */
            for (int k = 0; k < length && buf_idx < size - 1; k++)
                buf[buf_idx++] = buffer[k];

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
