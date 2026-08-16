#pragma once

/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "kernel/multiboot.h"
#include <stdint.h>

void module_callback(struct multiboot_parsed_module *mod, uint32_t index, void *);
