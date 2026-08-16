/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "kernel/core/mods.h"
#include "drivers/cpio.h"
#include "kernel/panic.h"
#include "kernel/qemu.h"
#include "libkern/string.h"

extern uint32_t _kernel_start;

void mod_cb(struct multiboot_parsed_module *mod, uint32_t idx, [[maybe_unused]] void *ctx) {
    const uint32_t mod_virt_start = (uint32_t)mod->start_addr;
    const char *cmdline_virt = (const char *)mod->cmdline;

    qemu_printf(QEMU_KERN, QEMU_INFO, "[MB] module %u: vaddr=%p size=%u cmd='%s'", idx, (void *)mod_virt_start,
                mod->size, cmdline_virt);

    /* Parse the initramfs payload */
    static const char initramfs_cmd[] = "initramfs";
    if (strcmp(cmdline_virt, initramfs_cmd) == 0) {
        struct cpio_info info;
        info.base_addr = (const uint8_t *)mod_virt_start;
        info.size = mod->size;

        if (cpio_init(info) != 0) {
            qemu_printf(QEMU_DRV, QEMU_PANIC, "[CPIO] init failed: parser setup returned an error");
            KERNEL_PANIC("CPIO parser", "Failed to initialize the CPIO parser");
        }
    }
}
