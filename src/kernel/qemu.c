/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "kernel/qemu.h"

#include "config.h"
#include "drivers/cmos.h"
#include "drivers/pit.h"
#include "libkern/bcd.h"
#include "libkern/string.h"

#include <stddef.h>

static void qemu_print(const char *s) {
    for (auto i = 0u; i < strlen(s); i++)
        qemu_putchar(s[i]);
}

static void qemu_print_time() {
    char buf[128] = {};
    ksnprintf(buf, sizeof(buf), "[ %lu.%03u ] ", (uint64_t)(pit_ticks / 1000), (uint32_t)(pit_ticks % 1000));
    qemu_print(buf);
}

static const char *qemu_cat_name(enum qemu_category cat) {
    switch (cat) {
    case QEMU_CPU:
        return "[CPU]";
    case QEMU_MEM:
        return "[MEM]";
    case QEMU_ARCH:
        return "[ARCH]";
    case QEMU_DRV:
        return "[DRV]";
    case QEMU_KERN:
        return "[KERN]";
    default:
        return "[UNK]";
    }
}

static const char *qemu_lvl_name(enum qemu_level lvl) {
    switch (lvl) {
    case QEMU_OK:
        return "[OK]";
    case QEMU_INFO:
        return "[INFO]";
    case QEMU_WARN:
        return "[WARN]";
    case QEMU_ERROR:
        return "[ERR]";
    case QEMU_PANIC:
        return "[PANIC]";
    default:
        return "[UNK]";
    }
}

void qemu_printf(enum qemu_category cat, enum qemu_level lvl, const char *fmt, ...) {
    qemu_print_time();
    qemu_print(qemu_cat_name(cat));
    qemu_print(" ");
    qemu_print(qemu_lvl_name(lvl));
    qemu_print(" ");

    va_list args;
    va_start(args, fmt);

    char buf[1024] = {};
    kvsnprintf(buf, sizeof(buf), fmt, args);
    qemu_print(buf);
    qemu_putchar('\n');

    va_end(args);
}
