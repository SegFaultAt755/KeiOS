#include "libkern/stdio.h"
#include "drivers/terminal.h"

int uvalue_to_str(char *buffer, unsigned int value, int base, int min_width, int zero_padding) {
    int i = 0;
    
    if (value == 0) {
        buffer[i++] = '0';
    } else {
        while (value > 0) {
            int rem = value % base;
            buffer[i++] = (rem < 10) ? (rem + '0') : (rem - 10 + 'a');
            value /= base;
        }
    }

    /* Apply zero padding */
    while (i < min_width && i < 32 && zero_padding) {
        buffer[i++] = '0';
    }

    /* Reverse the buffer */
    for (int j = 0; j < i / 2; j++) {
        char temp = buffer[j];
        buffer[j] = buffer[i - 1 - j];
        buffer[i - 1 - j] = temp;
    }

    return i; /* Returns length of the string */
}

void kprint_uint(unsigned int value, int base) {
    char buffer[32];
    int length = uvalue_to_str(buffer, value, base, 0, 0);
    for (int i = 0; i < length; i++)
        terminal_write(&buffer[i], 1);
}

void kprint_int(int value) {
    unsigned int uvalue = (unsigned int) value;
    if (value < 0) {
        terminal_writestring("-");
        uvalue = ~uvalue + 1;
    }

    kprint_uint(uvalue, 10);
}

void kvprintf(LogLevel level, const char *fmt, va_list args) {
    switch (level) {
        case LOG_INFO:    terminal_writestring("[INFO]: "); break;
        case LOG_WARNING: terminal_writestring("[WARNING]: "); break;
        case LOG_ERR:     terminal_writestring("[ERROR]: "); break;
        case LOG_DEBUG:
#if defined(DEBUG) && (DEBUG == true)
            terminal_writestring("[DEBUG]: "); 
            break;
#else
            return;
#endif
        default: break;
    }

    for (size_t i = 0; fmt[i] != '\0'; i++) {
        if (fmt[i] == '%' && fmt[i + 1] != '\0') {
            i++;

            int zero_padding = 0;
            int min_width = 0;

            if (fmt[i] == '0') {
                zero_padding = 1;
                i++;

                while (fmt[i] >= '0' && fmt[i] <= '9') {
                    min_width = min_width * 10 + (fmt[i] - '0');
                    i++;
                }
            }

            char buffer[32];
            int length = 0;

            switch (fmt[i]) {
                case 'd':
                case 'i': {
                    int value = va_arg(args, int);
                    unsigned int uvalue = (unsigned int) value;
                    if (value < 0) {
                        terminal_writestring("-");
                        uvalue = ~uvalue + 1;
                    }

                    length = uvalue_to_str(buffer, uvalue, 10, min_width, zero_padding);
                    for (int k = 0; k < length; k++) terminal_write(&buffer[k], 1);
                    break;
                }
                case 'u':
                    length = uvalue_to_str(buffer, va_arg(args, unsigned int), 10, min_width, zero_padding);
                    for (int k = 0; k < length; k++) terminal_write(&buffer[k], 1);
                    break;
                case 'x':
                    length = uvalue_to_str(buffer, va_arg(args, unsigned int), 16, min_width, zero_padding);
                    for (int k = 0; k < length; k++) terminal_write(&buffer[k], 1);
                    break;
                case 'o':
                    length = uvalue_to_str(buffer, va_arg(args, unsigned int), 8, min_width, zero_padding);
                    for (int k = 0; k < length; k++) terminal_write(&buffer[k], 1);
                    break;
                case 'b':
                    length = uvalue_to_str(buffer, va_arg(args, unsigned int), 2, min_width, zero_padding);
                    for (int k = 0; k < length; k++) terminal_write(&buffer[k], 1);
                    break;
                case 'p': {
                    void *ptr = va_arg(args, void *);
                    terminal_writestring("0x");

                    length = uvalue_to_str(buffer, (unsigned long long)(uintptr_t) ptr, 16, sizeof(void*) * 2, 1);
                    for (int k = 0; k < length; k++) terminal_write(&buffer[k], 1);
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

void kprintf(LogLevel level, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    kvprintf(level, fmt, args);
    va_end(args);
}

/* Safe string formatting function */
int ksnprintf(char *str, size_t size, const char *fmt, va_list args) {
    if (size == 0) return 0;

    size_t str_idx = 0;

    for (size_t i = 0; fmt[i] != '\0' && str_idx < size - 1; i++) {
        if (fmt[i] == '%' && fmt[i + 1] != '\0') {
            i++;
            char buffer[32];
            int length = 0;

            switch (fmt[i]) {
                case 'd':
                case 'i': {
                    int value = va_arg(args, int);
                    unsigned int uvalue = (unsigned int) value;
                    if (value < 0 && str_idx < size - 1) {
                        str[str_idx++] = '-';
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
                    if (str_idx < size - 1) {
                        str[str_idx++] = (char) va_arg(args, int);
                    }
                    continue;
                case 's': {
                    const char *s = va_arg(args, const char *);
                    if (!s) s = "(null)";
                    while (*s && str_idx < size - 1) {
                        str[str_idx++] = *s++;
                    }
                    continue;
                }
                case '%':
                    if (str_idx < size - 1) str[str_idx++] = '%';
                    continue;
                default:
                    if (str_idx < size - 1) str[str_idx++] = '%';
                    if (str_idx < size - 1) str[str_idx++] = fmt[i];
                    continue;
            }

            /* Copy formatted numerical buffers over */
            for (int k = 0; k < length && str_idx < size - 1; k++) {
                str[str_idx++] = buffer[k];
            }

        } else {
            str[str_idx++] = fmt[i];
        }
    }

    str[str_idx] = '\0'; /* Ensure null-termination */
    return (int) str_idx;
}
