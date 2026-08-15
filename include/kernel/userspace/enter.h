#pragma once

/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include <stdint.h>

#define USER_CS 0x1B
#define USER_DS 0x23

struct [[gnu::packed]] user_program_regs {
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
    uint32_t esp;
    uint32_t ss;
};

void execute_init_binary(void *cpio_binary, uint32_t binary_size);
