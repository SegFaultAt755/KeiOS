#pragma once

#include <stdarg.h>
#include "libkern/stdio.h"

#define QEMU_PORT 0xE9

typedef enum QemuLogLevel {
    QEMU_LOG_EMPTY,
    QEMU_LOG_INFO,
    QEMU_LOG_WARNING,
    QEMU_LOG_ERROR
} QemuLogLevel;

static inline void qemu_write_char(char c) {
    outb(QEMU_PORT, c);
}

void qemu_printf(QemuLogLevel level, const char *fmt, ...);