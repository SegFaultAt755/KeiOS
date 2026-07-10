#pragma once

/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include <stdint.h>
#include "arch/x86/isr.h"

void pit_initialize(uint32_t freq, void (*callback)(struct registers *regs));
