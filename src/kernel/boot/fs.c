/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "drivers/cpio.h"
#include "kernel/multiboot.h"
#include "kernel/qemu.h"

#include "kernel/core/cpio.h"
#include "kernel/core/mods.h"

/**
 * Load and parse kernel modules from the multiboot information.
 */
void boot_init_load_modules(struct multiboot_info *mbi_virt) {
    const uint32_t mod_count = mb_parse_mods(mbi_virt, mod_cb, nullptr);
    qemu_printf(QEMU_KERN, QEMU_INFO, "[Modules] Loaded %u module(s) at %p (flags=%u)", mod_count, mbi_virt,
                mbi_virt->flags);
}

/**
 * Parse and initialize the CPIO archive filesystem.
 */
void boot_init_cpio_filesystem(void) {
    cpio_parse(cpio_cb, nullptr);
}
