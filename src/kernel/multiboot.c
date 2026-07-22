/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "kernel/multiboot.h"

bool multiboot_has_modules(struct multiboot_info *mbi) {
    if (!mbi)
        return false;

    /* Check if bit 3 is set in the flags field */
    return (mbi->flags & MULTIBOOT_INFO_MODS) != 0;
}

uint32_t multiboot_parse_modules(struct multiboot_info *mbi, multiboot_module_callback_t callback, void *data) {
    if (!multiboot_has_modules(mbi) || mbi->mods_count == 0)
        return 0;

    struct multiboot_module *modules = (struct multiboot_module *)(uintptr_t)mbi->mods_addr;

    for (uint32_t i = 0; i < mbi->mods_count; i++) {
        struct multiboot_module *mod = &modules[i];

        if (callback) {
            struct multiboot_parsed_module parsed;
            parsed.start_addr = (const void *)(uintptr_t)mod->mod_start;
            parsed.end_addr = (const void *)(uintptr_t)mod->mod_end;
            parsed.size = (size_t)(mod->mod_end - mod->mod_start);
            parsed.cmdline = mod->cmdline ? (const char *)(uintptr_t)mod->cmdline : "";

            callback(&parsed, i, data);
        }
    }

    return mbi->mods_count;
}
