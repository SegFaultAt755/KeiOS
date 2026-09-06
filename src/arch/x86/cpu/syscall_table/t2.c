/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "arch/x86/syscall_table.h"
#include "kernel/shutdown.h"

[[noreturn]] uint32_t syscall_table_2([[maybe_unused]] struct syscall_registers *regs) {
    shutdown();
}
