/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "kernel/core/time.h"

void pit_cb([[maybe_unused]] struct registers *regs) {
    pit_ticks += 1;
}

void tick_wait(uint32_t ms) {
    sleep_ms(ms);
    pit_ticks += ms;
}
