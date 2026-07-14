/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "drivers/sleep.h"
#include "libkern/stdio.h"

void sleep_initialize(void) {
    /* Enable the pit channel 2 gate */
    uint8_t val = inb(0x61);
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
    for (uint32_t i = 0; i < ms; i++)
        sleep(1193); /* ~1ms worth of pit ticks */
}

void sleep_deinitialize(void) {
    /* Disable the pit channel 2 gate */
    outb(0x61, inb(0x61) & 0xFC);
}
