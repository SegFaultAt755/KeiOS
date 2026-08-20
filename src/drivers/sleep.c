/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "drivers/sleep.h"
#include "libkern/stdio.h"

constexpr uint16_t PIT_TICKS_1_MS = 1'193;

void sleep_init() {
    /* Enable the PIT channel 2 gate */
    auto val = inb(0x61);
    outb(0x61, (val & 0xFE) | 0x01);
}

void sleep(uint16_t ticks) {
    outb(0x43, 0xB0);

    /* The PIT base frequency is 1,193,182 Hz */
    /* A 1 ms delay needs about 1,193 PIT ticks */
    outb(0x42, (uint8_t)(ticks & 0xFF));
    outb(0x42, (uint8_t)((ticks >> 8) & 0xFF));

    /* This bit becomes high when channel 2 counts down to zero */
    while (!(inb(0x61) & 0x20)) {
        /* Wait until channel 2 finishes counting */
    }
}

void sleep_ms(uint32_t ms) {
    for (auto i = 0u; i < ms; i++)
        sleep(PIT_TICKS_1_MS); /* Wait about 1 ms of PIT ticks */
}

void sleep_deinitialize() {
    /* Disable the PIT channel 2 gate */
    outb(0x61, inb(0x61) & 0xFC);
}
