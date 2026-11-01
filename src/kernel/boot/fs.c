/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "drivers/cpio.h"
#include "kernel/multiboot.h"
#include "kernel/panic.h"
#include "kernel/qemu.h"

#include "kernel/core/cpio.h"
#include "kernel/core/mods.h"
#include "kernel/userspace/enter.h"

/**
 * Load and parse kernel modules from the multiboot information.
 */
void boot_init_load_modules(struct multiboot_info *mbi_virt) {
    KERNEL_ASSERT(mbi_virt != nullptr, "Invalid multiboot info",
                  "boot_init_load_modules called with NULL multiboot info - cannot load modules");

    const uint32_t mod_count = mb_parse_mods(mbi_virt, mod_cb, nullptr);

    KERNEL_ASSERT(mod_count <= 256, "Module count too large",
                  "mb_parse_mods returned unreasonably large module count - potential memory corruption or invalid "
                  "multiboot info");

    qemu_printf(QEMU_KERN, QEMU_INFO, "[Modules] Loaded %u module(s) at %p (flags=%u)", mod_count, mbi_virt,
                mbi_virt->flags);
}

/**
 * Parse and initialize the CPIO archive filesystem.
 */
void boot_init_cpio_filesystem(void) {
    cpio_status_t status = cpio_parse(cpio_cb, nullptr);
    KERNEL_ASSERT(status == CPIO_OK, "CPIO archive parsing failed",
                  "The initramfs contains malformed or truncated archive data");
}

void boot_init_run_init_program(void) {
    size_t binary_size;
    const uint8_t *binary = cpio_get_init_binary(&binary_size);
    if (!binary) {
        qemu_printf(QEMU_KERN, QEMU_WARN, "No bin/init.bin found in initramfs; continuing to kernel shell");
        return;
    }

    KERNEL_ASSERT(binary_size <= UINT32_MAX, "Init program is too large",
                  "The initramfs program does not fit in the i386 address space");
    execute_init_binary(binary, (uint32_t)binary_size);
}
