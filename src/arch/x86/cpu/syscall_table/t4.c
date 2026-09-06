/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "arch/x86/syscall_table.h"
#include "drivers/ipc.h"

uint32_t syscall_table_4(struct syscall_registers *regs) {
    return (uint32_t)ipc_send(regs->ebx, 0, (const uint8_t *)regs->ecx, regs->edx);
}