#pragma once

/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

inline void enable_interrupts() {
    __asm__ volatile ("sti");
}

inline void disable_interrupts() {
    __asm__ volatile ("cli");
}
