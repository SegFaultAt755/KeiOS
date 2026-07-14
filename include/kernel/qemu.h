#pragma once

/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include <stdarg.h>
#include "libkern/stdio.h"

#define QEMU_PORT 0xE9

enum qemu_level {
    QEMU_OK,
    QEMU_INFO,
    QEMU_WARN,
    QEMU_ERROR,
    QEMU_PANIC
};

enum qemu_category {
    QEMU_CPU,
    QEMU_MEM,
    QEMU_ARCH,
    QEMU_DRV,
    QEMU_KERN
};

static inline void qemu_putchar(char c) {
    outb(QEMU_PORT, c);
}

void qemu_set_time_var(uint32_t *ms);
void qemu_printf(enum qemu_category cat, enum qemu_level level, const char *fmt, ...);