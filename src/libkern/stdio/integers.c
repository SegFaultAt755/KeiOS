#include "libkern/stdio.h"
#include "libkern/string.h"
#include "drivers/terminal.h"

void kprint_uint(unsigned int val, int base) {
    char buf[32];
    int len = uvalue_to_str(buf, val, base, 0, 0);
    for (int i = 0; i < len; i++)
        terminal_write(&buf[i], 1);
}

void kprint_int(int val) {
    unsigned int uval = (unsigned int) val;
    if (val < 0) {
        terminal_putchar('-');
        uval = ~uval + 1;
    }

    kprint_uint(uval, 10);
}
