/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "drivers/display.h"
#include "drivers/graphics/framebuffer.h"
#include "drivers/ps2.h"
#include "drivers/terminal.h"
#include "drivers/vga.h"
#include "kernel/ipc.h"
#include "kernel/multiboot.h"
#include "kernel/panic.h"
#include "kernel/pit.h"
#include "kernel/qemu.h"

#include "kernel/core/time.h"

static void boot_init_vga_terminal(void) {
    vga_init_text();
    terminal_init((uint16_t *)VGA_TEXT_MEMORY, VGA_TEXT_WIDTH, VGA_TEXT_HEIGHT);
}

static void boot_init_display(struct multiboot_info *mbi) {
    if ((mbi->flags & MULTIBOOT_INFO_FRAMEBUFFER_INFO) == 0 ||
        mbi->framebuffer_type != MULTIBOOT_FRAMEBUFFER_TYPE_RGB) {
        boot_init_vga_terminal();
        return;
    }

    bool initialized = framebuffer_init(mbi) == 0 && display_init();

    if (initialized) {
        display_clear(0);
        qemu_printf(QEMU_DRV, QEMU_OK, "Framebuffer display initialized: (width: %d, height: %d)",
                    mbi->framebuffer_width, mbi->framebuffer_height);
        return;
    }

    qemu_printf(QEMU_DRV, QEMU_ERROR, "Failed to initialize framebuffer display; falling back to VGA text mode");
    boot_init_vga_terminal();
}

/**
 * Initialize all hardware drivers (PIT, PS/2, display).
 */
void boot_init_hardware_drivers(struct multiboot_info *mbi_virt) {
    KERNEL_ASSERT(mbi_virt != nullptr, "Invalid multiboot info",
                  "boot_init_hardware_drivers called with NULL multiboot info");

    KERNEL_ASSERT(ipc_init() == IPC_OK, "IPC initialization failed", "Unable to initialize the kernel IPC engine");

    auto keyboard_endpoint = ipc_endpoint_create();
    KERNEL_ASSERT(keyboard_endpoint != 0, "IPC endpoint allocation failed",
                  "Unable to allocate the PS/2 keyboard IPC endpoint");

    pit_init(1193, pit_callback);

    ps2_set_ipc_endpoint(keyboard_endpoint);
    ps2_init();

    boot_init_display(mbi_virt);
}
