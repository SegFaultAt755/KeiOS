#pragma once

/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include <stdint.h>

/* PS/2 Controller Ports */
#define PS2_DATA_PORT    0x60
#define PS2_STATUS_PORT  0x64

/* PS/2 Controller Commands */
#define PS2_CMD_READ_CONFIG   0x20
#define PS2_CMD_WRITE_CONFIG  0x60
#define PS2_CMD_SELF_TEST     0xAA
#define PS2_CMD_IF_TEST       0xAB
#define PS2_CMD_ENABLE_KBD    0xAE
#define PS2_CMD_DISABLE_KBD   0xAD

/* PS/2 Controller Config Byte Bits */
#define PS2_CFG_IRQ1_ENABLED  (1U << 0)
#define PS2_CFG_MOUSE_IRQ     (1U << 1)
#define PS2_CFG_CLOCKDisabled (1U << 4)

/* Modifier key scancodes (Set 1 make codes) */
#define SC_LSHIFT   0x2A
#define SC_RSHIFT   0x36
#define SC_LCTRL    0x1D
#define SC_LALT     0x38
#define SC_CAPSLOCK 0x3A

/* Extended scancode prefix */
#define SC_EXTENDED 0xE0

/* Modifier bit flags */
#define MOD_LSHIFT   (1U << 0)
#define MOD_RSHIFT   (1U << 1)
#define MOD_LCTRL    (1U << 2)
#define MOD_LALT     (1U << 3)
#define MOD_CAPSLOCK (1U << 4)

void ps2_initialize(void);
void ps2_disable(void);
uint8_t ps2_get_modifiers(void);
