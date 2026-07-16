/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "kernel/qemu.h"

#include <stddef.h>

#include "config.h"
#include "drivers/cmos.h"
#include "libkern/bcd.h"
#include "libkern/string.h"

static void qemu_print(const char *s) {
    for (size_t i = 0; i < strlen(s); i++)
        qemu_putchar(s[i]);
}

static uint32_t *var_ms_track = nullptr;
static void qemu_print_time(void) {
    char buf[1024] = {};

    if (var_ms_track != nullptr)
        ksnprintf(buf, sizeof(buf), "[ %d.%d ] ", *var_ms_track / 1000, *var_ms_track % 1000);
    else
        ksnprintf(buf, sizeof(buf), "[ UNDEFI ] ");

    qemu_print(buf);
}

void qemu_set_time_var(uint32_t *ms) {
    var_ms_track = ms;
}

void qemu_printf(enum qemu_category cat, enum qemu_level level, const char *fmt, ...) {
    qemu_print_time();

    switch (cat) {
    case QEMU_CPU: {
        qemu_print("[CPU ] ");
    } break;
    case QEMU_MEM: {
        qemu_print("[MEM ] ");
    } break;
    case QEMU_ARCH: {
        qemu_print("[ARCH] ");
    } break;
    case QEMU_DRV: {
        qemu_print("[DRV ] ");
    } break;
    case QEMU_KERN: {
        qemu_print("[KERN] ");
    } break;
    default:
        break;
    }

    switch (level) {
    case QEMU_OK: {
        qemu_print("[ OK ] ");
    } break;
    case QEMU_INFO: {
        qemu_print("[INFO] ");
    } break;
    case QEMU_WARN: {
        qemu_print("[WARN] ");
    } break;
    case QEMU_ERROR: {
        qemu_print("[ERR ] ");
    } break;
    case QEMU_PANIC: {
        qemu_print("[PANC] ");
    } break;
    default:
        break;
    }

    va_list args;
    va_start(args, fmt);

    char buf[1024] = {};
    kvsnprintf(buf, sizeof(buf), fmt, args);
    qemu_print(buf);
    qemu_putchar('\n');

    va_end(args);
}
