/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "arch/x86/syscall_table.h"

uint32_t syscall_table_2([[maybe_unused]] struct syscall_registers *regs) {
    /* Shutdown not implemented */
    return 0xDEADC0DE;
}
