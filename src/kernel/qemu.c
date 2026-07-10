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

static void qemu_print_time(void) {
    char buf[1024] = {};
    ksnprintf(buf, sizeof(buf), "[%d-%d-%d]: ", bcd_to_binary(read_cmos_reg(CMOS_HOUR)),
              bcd_to_binary(read_cmos_reg(CMOS_MIN)), bcd_to_binary(read_cmos_reg(CMOS_SEC)));

    qemu_print(buf);
}

void qemu_printf(enum qemu_log level, const char *fmt, ...) {
    switch (level) {
    case QEMU_INFO: {
        qemu_print("[INFO]: ");
        qemu_print_time();
    } break;
    case QEMU_WARN: {
        qemu_print("[WARN]: ");
        qemu_print_time();
    } break;
    case QEMU_ERROR: {
        qemu_print("[ERROR]: ");
        qemu_print_time();
    } break;
    case QEMU_PANIC: {
        qemu_print("[PANIC]: ");
        qemu_print_time();
    } break;
    default:
        break;
    }

    va_list args;
    va_start(args, fmt);

    char buf[1024] = {};
    ksnprintf(buf, sizeof(buf), fmt, args);
    qemu_print(buf);
    qemu_putchar('\n');

    va_end(args);
}
