#pragma once

/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "drivers/cpio.h"
#include <stdint.h>
#include <stddef.h>

extern uint8_t *exec_init;
extern uint32_t exec_init_size;

void cpio_callback_function(const char *name, struct cpio_header header, const uint8_t *data, size_t data_len, void *user_context);
