/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "drivers/cmos.h"

constexpr uint8_t CMOS_ADDR = 0x70;
constexpr uint8_t CMOS_DATA = 0x71;

uint8_t read_cmos_reg(uint8_t reg) {
    outb(CMOS_ADDR, reg);
    return inb(CMOS_DATA);
}
