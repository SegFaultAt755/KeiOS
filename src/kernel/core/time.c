/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "kernel/core/time.h"

void pit_callback([[maybe_unused]] struct registers *regs) {
    pit_ticks += 1;
}
