/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "drivers/ps2.h"

#include "arch/x86/isr.h"
#include "kernel/qemu.h"
#include "libkern/stdio.h"

/* Scancode Set 1 (unshifted) */
static const char sc1_unshifted[128] = {
    0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', 0,   'q', 'w', 'e',  'r', 't', 'y',
    'u', 'i', 'o', 'p', '[', ']', '\n', 0,   'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,   '\\',
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,    0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   '-', 0,   0,   0,   '+', 0,   0,   0,   0,   0,   0,    0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,    0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0};

/* Scancode Set 1 (shifted) */
static const char sc1_shifted[128] = {
    0,   0,   '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 0,   0,   'Q', 'W', 'E', 'R', 'T', 'Y',
    'U', 'I', 'O', 'P', '{', '}', 0,   0,   'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0,   '|',
    'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   '-', 0,   0,   0,   '+', 0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0};

static uint8_t modifier_state = 0;
static uint8_t extended_prefix = 0;
static void (*key_callback)(uint16_t) = nullptr;

static int is_letter(uint8_t sc) {
    return (sc >= 0x10 && sc <= 0x19) || (sc >= 0x1E && sc <= 0x26) || (sc >= 0x2C && sc <= 0x32);
}

static void update_modifiers(uint8_t scancode) {
    switch (scancode) {
    case SC_LSHIFT:
        modifier_state |= MOD_LSHIFT;
        break;
    case SC_RSHIFT:
        modifier_state |= MOD_RSHIFT;
        break;
    case SC_LCTRL:
        modifier_state |= MOD_LCTRL;
        break;
    case SC_LALT:
        modifier_state |= MOD_LALT;
        break;
    case SC_CAPSLOCK:
        modifier_state ^= MOD_CAPSLOCK;
        break;
    }
}

static void clear_modifiers(uint8_t scancode) {
    switch (scancode) {
    case 0xAA:
        modifier_state &= ~MOD_LSHIFT;
        break;
    case 0xB6:
        modifier_state &= ~MOD_RSHIFT;
        break;
    case 0x9D:
        modifier_state &= ~MOD_LCTRL;
        break;
    case 0xB8:
        modifier_state &= ~MOD_LALT;
        break;
    }
}

static void keyboard_handler(struct registers *regs) {
    (void)regs;
    uint8_t scancode = inb(PS2_DATA_PORT);

    /* Discard error bytes from controller */
    if (scancode == 0x00 || scancode == 0xE1)
        return;

    /* Extended scancode prefix — discard, next IRQ gets the real byte */
    if (scancode == SC_EXTENDED) {
        extended_prefix = 1;
        return;
    }

    /* Handle extended scancodes (arrows, Right Ctrl/Alt, etc.) */
    if (extended_prefix) {
        extended_prefix = 0;
        if (scancode & 0x80) {
            /* Extended break code — clear extended modifiers */
            switch (scancode) {
            case 0x9D:
                modifier_state &= ~MOD_RCTRL;
                break;
            case 0xB8:
                modifier_state &= ~MOD_RALT;
                break;
            }
        } else {
            /* Extended make code — handle arrow keys and modifiers */
            switch (scancode) {
            case SC_EXT_UP:
                if (key_callback)
                    key_callback(KEY_UP);
                break;
            case SC_EXT_DOWN:
                if (key_callback)
                    key_callback(KEY_DOWN);
                break;
            case SC_EXT_LEFT:
                if (key_callback)
                    key_callback(KEY_LEFT);
                break;
            case SC_EXT_RIGHT:
                if (key_callback)
                    key_callback(KEY_RIGHT);
                break;
            case SC_EXT_RCTRL:
                modifier_state |= MOD_RCTRL;
                break;
            case SC_EXT_RALT:
                modifier_state |= MOD_RALT;
                break;
            }
        }
        return;
    }

    /* Break code (key release) */
    if (scancode & 0x80) {
        clear_modifiers(scancode);
        return;
    }

    /* Make code (key press) */
    update_modifiers(scancode);

    /* Determine if shift is active for this key */
    uint8_t shift = (modifier_state & (MOD_LSHIFT | MOD_RSHIFT)) != 0;
    if (is_letter(scancode) && (modifier_state & MOD_CAPSLOCK))
        shift = !shift;

    char ascii = shift ? sc1_shifted[scancode] : sc1_unshifted[scancode];

    if (ascii && key_callback)
        key_callback(ascii);
}

void ps2_initialize(void) {
    /* Flush output buffer */
    while (inb(PS2_STATUS_PORT) & 0x01)
        inb(PS2_DATA_PORT);

    /* Disable keyboard */
    outb(PS2_STATUS_PORT, PS2_CMD_DISABLE_KBD);
    waitb(1);

    /* Read config byte */
    outb(PS2_STATUS_PORT, PS2_CMD_READ_CONFIG);
    waitb(1);
    uint8_t cfg = inb(PS2_DATA_PORT);

    /* Enable IRQ1, disable mouse IRQ */
    cfg = (cfg | PS2_CFG_IRQ1_ENABLED) & ~PS2_CFG_MOUSE_IRQ;
    outb(PS2_STATUS_PORT, PS2_CMD_WRITE_CONFIG);
    waitb(1);
    outb(PS2_DATA_PORT, cfg);

    /* Controller self-test */
    outb(PS2_STATUS_PORT, PS2_CMD_SELF_TEST);
    waitb(1);
    uint8_t result = inb(PS2_DATA_PORT);
    if (result != 0x55)
        qemu_printf(QEMU_DRV, QEMU_ERROR, "PS/2 controller self-test result: 0x%x | must be 0x55", result);

    /* Interface test */
    outb(PS2_STATUS_PORT, PS2_CMD_IF_TEST);
    waitb(1);
    result = inb(PS2_DATA_PORT);
    if (result != 0)
        qemu_printf(QEMU_DRV, QEMU_ERROR, "PS/2 interface self-test result: %d | must be 0", result);

    /* Enable keyboard */
    outb(PS2_STATUS_PORT, PS2_CMD_ENABLE_KBD);
    waitb(1);

    /* Register IRQ1 handler */
    intr_handler(IRQ1, keyboard_handler);
    qemu_printf(QEMU_DRV, QEMU_OK, "PS/2 Keyboard initialized");
}

void ps2_disable(void) {
    intr_handler(IRQ1, 0);
    outb(PS2_STATUS_PORT, PS2_CMD_DISABLE_KBD);
}

uint8_t ps2_get_modifiers(void) {
    return modifier_state;
}

void ps2_set_key_callback(void (*cb)(uint16_t)) {
    key_callback = cb;
}
