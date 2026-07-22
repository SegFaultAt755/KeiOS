/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "kernel/graphics.h"

int get_graphics_type(struct multiboot_info *mbi) {
    if (mbi->flags & MULTIBOOT_INFO_FRAMEBUFFER_INFO) {
        if (mbi->framebuffer_type == GRAPHICS_TYPE_FRAMEBUFFER)
            return GRAPHICS_TYPE_FRAMEBUFFER;

        return GRAPHICS_TYPE_TEXT_MODE;
    }

    return GRAPHICS_TYPE_VGA_PALETTE;
}
