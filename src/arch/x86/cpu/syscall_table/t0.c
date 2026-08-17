/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "arch/x86/syscall_table.h"
#include "config.h"

uint32_t syscall_table_0([[maybe_unused]] struct syscall_registers *regs) {
    return ((uint8_t)VERSION_MAJOR) | (((uint8_t)VERSION_MINOR) << 8) | (((uint8_t)VERSION_PATCH) << 16);
}
