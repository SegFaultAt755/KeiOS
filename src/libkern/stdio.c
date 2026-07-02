#include "libkern/stdio.h"
#include "drivers/terminal.h"
#include <stddef.h>

void kprint_uint(unsigned int val, int base) {
    char buf[32];
    int i = 0;
    
    if (val == 0) {
        terminal_writestring("0");
        return;
    }
    
    while (val > 0) {
        int rem = val % base;
        buf[i++] = (rem < 10) ? (rem + '0') : (rem - 10 + 'a');
        val /= base;
    }
    
    while (i > 0) {
        char c = buf[--i];
        terminal_write(&c, 1);
    }
}

void kprint_int(int val) {
    unsigned int uval = (unsigned int) val;
    if (val < 0) {
        terminal_writestring("-");
        uval = ~uval + 1;
    }

    kprint_uint(uval, 10);
}

void kprintf(LogLevel level, const char *fmt, ...) {
    switch (level) {
        case LOG_INFO:  terminal_writestring("[INFO]: "); break;
        case LOG_WARN:  terminal_writestring("[WARN]: "); break;
        case LOG_ERR:   terminal_writestring("[ERROR]: "); break;
        case LOG_DEBUG: terminal_writestring("[DEBUG]: "); break;
        default: break;
    }

    va_list args;
    va_start(args, fmt);

    for (size_t i = 0; fmt[i] != '\0'; i++) {
        if (fmt[i] == '%' && fmt[i + 1] != '\0') {
            i++;
            
        switch (fmt[i]) {
            case 'd':
            case 'i':
                kprint_int(va_arg(args, int));
                break;
            case 'u':
                kprint_uint(va_arg(args, unsigned int), 10);
                break;
            case 'x':
                kprint_uint(va_arg(args, unsigned int), 16);
                break;
            case 'c': {
                /* chars are promoted to int when passed through '...' */
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
                /* Unknown format specifier, just print it as is */
                terminal_writestring("%");
                terminal_write(&fmt[i], 1);
                break;
            }
        } else {
            /* Standard character, print directly */
            terminal_write(&fmt[i], 1);
        }
    }

    va_end(args);
}