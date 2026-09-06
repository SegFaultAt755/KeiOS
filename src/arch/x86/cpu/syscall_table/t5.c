/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "arch/x86/syscall_table.h"
#include "drivers/ipc.h"

uint32_t syscall_table_5(struct syscall_registers *regs) {
    return (uint32_t)ipc_receive(regs->ebx, (struct ipc_message *)regs->ecx);
}