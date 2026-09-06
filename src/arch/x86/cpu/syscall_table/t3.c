/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "arch/x86/syscall_table.h"
#include "drivers/ipc.h"

uint32_t syscall_table_3([[maybe_unused]] struct syscall_registers *regs) {
    return ipc_endpoint_create();
}