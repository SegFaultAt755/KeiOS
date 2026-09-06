#pragma once

/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include <stdint.h>

#define SYSCALL_VERSION              0
#define SYSCALL_REBOOT               1
#define SYSCALL_SHUTDOWN             2
#define SYSCALL_IPC_ENDPOINT_CREATE  3
#define SYSCALL_IPC_SEND             4
#define SYSCALL_IPC_RECEIVE          5

struct syscall_registers {
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
};

void syscall_stub(void);
void syscall_handler(struct syscall_registers *regs);
