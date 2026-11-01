/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "arch/x86/syscall.h"
#include "arch/x86/syscall_table.h"
#include "kernel/reboot.h"

static uint32_t parse_codes(struct syscall_registers *regs) {
#define X(n)                                                                                                           \
    if (regs->eax == n)                                                                                                \
        return syscall_table_##n(regs);
    SYSCALL_TABLE_LIST
#undef X

    return 0xDEADC0DE;
}

uint32_t syscall_handler(struct syscall_registers *regs) {
    uint32_t syscall_number = regs->eax;
    regs->eax = parse_codes(regs);
    return syscall_number == SYSCALL_EXIT ? SYSCALL_RETURN_TO_KERNEL : 0;
}
