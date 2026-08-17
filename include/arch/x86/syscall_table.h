#pragma once

/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include <stdint.h>
#include "arch/x86/syscall.h"

#define SYSCALL_TABLE_LIST \
    X(0) X(1) X(2)

#define X(n) uint32_t syscall_table_##n(struct syscall_registers *regs);
    SYSCALL_TABLE_LIST
#undef X
