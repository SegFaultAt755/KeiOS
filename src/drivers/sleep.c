/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "drivers/sleep.h"
#include "libkern/stdio.h"

constexpr uint16_t PIT_TICKS_1_MS = 1'193;

void sleep_initialize() {
    /* Enable the pit channel 2 gate */
    auto val = inb(0x61);
    outb(0x61, (val & 0xFE) | 0x01);
}

void sleep(uint16_t ticks) {
    outb(0x43, 0xB0);

    /* PIT base frequency is 1193182 hz */
    /* To wait 1ms, use a tick count of 1193 (approx 1193182 / 1000) */
    outb(0x42, (uint8_t)(ticks & 0xFF));
    outb(0x42, (uint8_t)((ticks >> 8) & 0xFF));

    /* Bit goes high when channel 2 finishes counting down to 0 */
    while (!(inb(0x61) & 0x20)) {
        /* Waiting... */
    }
}

void sleep_ms(uint32_t ms) {
    for (auto i = 0u; i < ms; i++)
        sleep(PIT_TICKS_1_MS); /* ~1ms worth of pit ticks */
}

void sleep_deinitialize() {
    /* Disable the pit channel 2 gate */
    outb(0x61, inb(0x61) & 0xFC);
}
