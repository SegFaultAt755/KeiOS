#pragma once

/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "kernel/multiboot.h"

#define GRAPHICS_TYPE_VGA_PALETTE 2
#define GRAPHICS_TYPE_FRAMEBUFFER 1
#define GRAPHICS_TYPE_TEXT_MODE   0

int get_graphics_type(struct multiboot_info *mbi);
