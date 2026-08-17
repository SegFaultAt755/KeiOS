/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "arch/x86/syscall_table.h"
#include "kernel/reboot.h"

[[noreturn]] uint32_t syscall_table_1([[maybe_unused]] struct syscall_registers *regs) {
    if (regs->ebx != 0)
        reboot_raw();

    reboot();

    while (true) {
    }
}
