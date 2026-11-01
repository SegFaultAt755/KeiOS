/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "arch/x86/heap.h"
#include "kernel/panic.h"
#include "kernel/qemu.h"

#include <setjmp.h>
#include <stdlib.h>

static jmp_buf panic_env;
static bool expect_panic;

[[noreturn]] void runtime_panic(const char *reason, const char *desc, const char *file, uint32_t line) {
    (void)reason;
    (void)desc;
    (void)file;
    (void)line;
    if (expect_panic) {
        longjmp(panic_env, 1);
    }
    abort();
}

[[noreturn]] void runtime_panic_format(const char *reason, const char *desc, const char *file, uint32_t line, ...) {
    (void)reason;
    (void)desc;
    (void)file;
    (void)line;
    if (expect_panic) {
        longjmp(panic_env, 1);
    }
    abort();
}

void qemu_printf(enum qemu_category category, enum qemu_level level, const char *format, ...) {
    (void)category;
    (void)level;
    (void)format;
}

int main(void) {
    _Alignas(16) uint8_t storage[4096];
    heap_init(storage, sizeof(storage));

    void *first = kmalloc(1);
    void *second = kmalloc(17);
    if (((uintptr_t)first & 15U) != 0 || ((uintptr_t)second & 15U) != 0) {
        return 1;
    }

    if (setjmp(panic_env) == 0) {
        expect_panic = true;
        kfree((uint8_t *)first + 16);
        return 2;
    }
    expect_panic = false;

    kfree(first);
    kfree(second);
    if (kmalloc(4000) == nullptr) {
        return 3;
    }

    return 0;
}
