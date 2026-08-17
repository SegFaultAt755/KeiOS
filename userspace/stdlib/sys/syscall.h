#pragma once

/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include <stdint.h>

uint32_t syscall(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx);
