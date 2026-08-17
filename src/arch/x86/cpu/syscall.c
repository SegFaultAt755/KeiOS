/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "arch/x86/syscall.h"
#include "kernel/reboot.h"

static uint32_t parse_codes(uint32_t eax, uint32_t ebx) {
    switch (eax) {
        case 0: {reboot(); return 0xDEADC0DE;}
        default: return 0xDEADC0DE;
    };
}

[[gnu::naked]] void syscall_handler(void) {
    uint32_t ceax, cebx;

    __asm__ volatile (
        "mov %%eax, %0;\n\t"
        "mov %%ebx, %1;"
        : "=r" (ceax), "=r" (cebx)
    );

    uint32_t err_code = parse_codes(ceax, cebx);
    __asm__ volatile (
        "mov $0, %%eax"
        :
        : "r" (err_code)
        : "eax"
    );

    __asm__ volatile ("iret");
}
