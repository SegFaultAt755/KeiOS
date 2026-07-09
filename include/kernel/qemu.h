#pragma once

#include <stdarg.h>
#include "libkern/stdio.h"

#define QEMU_PORT 0xE9

enum qemu_log {
    QEMU_LOG_EMPTY,
    QEMU_LOG_INFO,
    QEMU_LOG_WARNING,
    QEMU_LOG_ERROR,
    QEMU_LOG_PANIC
};

static inline void qemu_write_char(char c) {
    outb(QEMU_PORT, c);
}

void qemu_printf(enum qemu_log level, const char *fmt, ...);