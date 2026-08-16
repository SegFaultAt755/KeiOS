/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "kernel/core/multiboot_modules.h"
#include "drivers/cpio.h"
#include "kernel/panic.h"
#include "kernel/qemu.h"
#include "libkern/string.h"

extern uint32_t _kernel_start;

void module_callback(struct multiboot_parsed_module *mod, uint32_t index, void *) {
    uint32_t mod_virt_start = (uint32_t)mod->start_addr;
    const char *cmdline_virt = (const char *)mod->cmdline;

    qemu_printf(QEMU_KERN, QEMU_INFO, "Module %u: (virt_start=%p, size=%u bytes, cmd='%s')", 
                index, (void*)mod_virt_start, mod->size, cmdline_virt);

    /* Parse CPIO */
    const char *initramfs_cmd = "initramfs";
    if (strcmp(cmdline_virt, initramfs_cmd) == 0) {
        struct cpio_info info;
        info.base_addr = (const uint8_t *)mod_virt_start; 
        info.size = mod->size;

        if (cpio_initialize(info) != 0) { 
            qemu_printf(QEMU_DRV, QEMU_PANIC, "Failed to initialize CPIO parser");
            KERNEL_PANIC("CPIO Parser", "Failed to initialize CPIO parser");
        }
    }
}
