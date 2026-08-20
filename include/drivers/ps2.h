#pragma once

/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include <stdint.h>

/* PS/2 controller ports */
#define PS2_DATA_PORT    0x60
#define PS2_STATUS_PORT  0x64

/* PS/2 controller commands */
#define PS2_CMD_READ_CONFIG   0x20
#define PS2_CMD_WRITE_CONFIG  0x60
#define PS2_CMD_SELF_TEST     0xAA
#define PS2_CMD_IF_TEST       0xAB
#define PS2_CMD_ENABLE_KBD    0xAE
#define PS2_CMD_DISABLE_KBD   0xAD

/* Bits in the PS/2 controller configuration byte */
#define PS2_CFG_IRQ1_ENABLED  (1U << 0)
#define PS2_CFG_MOUSE_IRQ     (1U << 1)
#define PS2_CFG_CLOCKDisabled (1U << 4)

/* Modifier key scancodes in set 1 make-code format */
#define SC_LSHIFT   0x2A
#define SC_RSHIFT   0x36
#define SC_LCTRL    0x1D
#define SC_LALT     0x38
#define SC_CAPSLOCK 0x3A

/* Prefix used by extended scancodes */
#define SC_EXTENDED 0xE0

/* Extended make codes that follow the 0xE0 prefix */
#define SC_EXT_UP      0x48
#define SC_EXT_DOWN    0x50
#define SC_EXT_LEFT    0x4B
#define SC_EXT_RIGHT   0x4D
#define SC_EXT_RCTRL   0x1D
#define SC_EXT_RALT    0x38

/* Special key codes returned by the callback; these have no ASCII equivalent */
#define KEY_UP     0x100
#define KEY_DOWN   0x101
#define KEY_LEFT   0x102
#define KEY_RIGHT  0x103

/* Modifier-key bit flags */
#define MOD_LSHIFT   (1U << 0)
#define MOD_RSHIFT   (1U << 1)
#define MOD_LCTRL    (1U << 2)
#define MOD_LALT     (1U << 3)
#define MOD_CAPSLOCK (1U << 4)
#define MOD_RCTRL    (1U << 5)
#define MOD_RALT     (1U << 6)

void ps2_init(void);
void ps2_disable(void);
uint8_t ps2_get_modifiers(void);
void ps2_set_key_callback(void (*cb)(uint16_t));
