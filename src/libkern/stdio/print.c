/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "drivers/terminal.h"
#include "libkern/stdio.h"
#include "libkern/string.h"

void kvprintf(const char *fmt, va_list args) {
    char buf[1024];
    int len = kvsnprintf(buf, sizeof(buf), fmt, args);

    if (len > 0)
        terminal_write(buf, len);
}

void kprintf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    kvprintf(fmt, args);
    va_end(args);
}
