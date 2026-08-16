/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "kernel/multiboot.h"

#define HIGHER_HALF_OFFSET 0xC000'0000U

[[nodiscard]] bool mb_has_mods(struct multiboot_info *mbi) {
    if (!mbi)
        return false;

    /* Check if bit 3 is set in the flags field */
    return (mbi->flags & MULTIBOOT_INFO_MODS) != 0;
}

[[nodiscard]] uint32_t mb_parse_mods(struct multiboot_info *mbi, multiboot_module_callback_t cb, void *data) {
    if (!mbi)
        return 0;

    if ((uintptr_t)mbi < HIGHER_HALF_OFFSET) {
        mbi = (struct multiboot_info *)((uintptr_t)mbi + HIGHER_HALF_OFFSET);
    }

    if (!mb_has_mods(mbi) || mbi->mods_count == 0)
        return 0;

    /* Translate physical mods_addr array to higher-half virtual address */
    auto mods = (struct multiboot_module *)((uintptr_t)mbi->mods_addr + HIGHER_HALF_OFFSET);

    for (auto i = 0u; i < mbi->mods_count; i++) {
        auto mod = &mods[i];

        if (cb) {
            struct multiboot_parsed_module parsed;
            parsed.start_addr = (const void *)((uintptr_t)mod->mod_start + HIGHER_HALF_OFFSET);
            parsed.end_addr = (const void *)((uintptr_t)mod->mod_end + HIGHER_HALF_OFFSET);
            parsed.size = (size_t)(mod->mod_end - mod->mod_start);

            /* Translate physical cmdline pointer to higher-half virtual address */
            parsed.cmdline = mod->cmdline ? (const char *)((uintptr_t)mod->cmdline + HIGHER_HALF_OFFSET) : "";

            cb(&parsed, i, data);
        }
    }

    return mbi->mods_count;
}
