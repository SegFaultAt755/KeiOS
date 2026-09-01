/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "drivers/pit.h"
#include "drivers/ps2.h"
#include "kernel/graphics.h"
#include "kernel/multiboot.h"
#include "kernel/panic.h"

#include "kernel/core/time.h"

/**
 * Initialize all hardware drivers (PIT, PS2, graphics).
 */
void boot_init_hardware_drivers(struct multiboot_info *mbi_virt) {
    KERNEL_ASSERT(mbi_virt != nullptr, "Invalid multiboot info",
                  "boot_init_hardware_drivers called with NULL multiboot info - cannot initialize graphics without "
                  "framebuffer info");

    pit_init(1193, pit_cb);
    ps2_init();
    gfx_init(mbi_virt);
}
