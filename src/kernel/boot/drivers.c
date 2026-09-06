/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "drivers/ipc.h"
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

    KERNEL_ASSERT(ipc_init() == IPC_OK, "IPC initialization failed", "Unable to initialize the kernel IPC engine");

    auto keyboard_endpoint = ipc_endpoint_create();
    KERNEL_ASSERT(keyboard_endpoint != 0, "IPC endpoint allocation failed",
                  "Unable to allocate the PS/2 keyboard IPC endpoint");

    ps2_set_ipc_endpoint(keyboard_endpoint);
    pit_init(1193, pit_cb);
    ps2_init();
    gfx_init(mbi_virt);
}
