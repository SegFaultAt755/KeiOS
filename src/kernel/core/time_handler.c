/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "kernel/core/time_handler.h"

void pit_callback(struct registers *) {
    pit_ticks += 1;
}

void tick_wait(uint32_t ms) {
    sleep_ms(ms);
    pit_ticks += ms;
}
