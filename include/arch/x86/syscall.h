#pragma once

/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include <stdint.h>

struct syscall_registers {
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
};

void syscall_stub(void);
void syscall_handler(struct syscall_registers *regs);
