#pragma once

/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "drivers/cpio.h"
#include <stddef.h>
#include <stdint.h>

void cpio_cb(const char *name, struct cpio_header hdr, const uint8_t *data, size_t data_len, void *ctx);
