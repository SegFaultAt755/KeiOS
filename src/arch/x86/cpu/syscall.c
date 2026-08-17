/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "arch/x86/syscall.h"

[[gnu::naked]] void syscall_handler(void) {
    __asm__ volatile (
        "mov $999, %%eax"
        ::: "eax"
    );

    __asm__ volatile ("iret");
}
