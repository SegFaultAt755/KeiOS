/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "kernel/multiboot.h"

#if defined(__i386__) || defined(_M_IX86)
#include "arch/x86/mem.h"
#else
#error "Unsupported architecture! (i386 is available)"
#endif

[[nodiscard]] bool mb_has_mods(struct multiboot_info *mbi) {
    if (!mbi)
        return false;

    /* Check whether bit 3 is set in the flags field */
    return (mbi->flags & MULTIBOOT_INFO_MODS) != 0;
}

[[nodiscard]] uint32_t mb_parse_mods(struct multiboot_info *mbi, multiboot_module_callback_t cb, void *data) {
    if (!mbi)
        return 0;

    if ((uintptr_t)mbi < KERNEL_VIRTUAL_OFFSET) {
        mbi = (struct multiboot_info *)ADD_KERNEL_OFFSET((uintptr_t)mbi);
    }

    if (!mb_has_mods(mbi) || mbi->mods_count == 0)
        return 0;

    /* Convert the physical module list address to a higher-half virtual address */
    auto mods = (struct multiboot_module *)ADD_KERNEL_OFFSET((uintptr_t)mbi->mods_addr);

    for (auto i = 0u; i < mbi->mods_count; i++) {
        auto mod = &mods[i];

        if (cb) {
            struct multiboot_parsed_module parsed;
            parsed.start_addr = (const void *)ADD_KERNEL_OFFSET((uintptr_t)mod->mod_start);
            parsed.end_addr = (const void *)ADD_KERNEL_OFFSET((uintptr_t)mod->mod_end);
            parsed.size = (size_t)(mod->mod_end - mod->mod_start);

            /* Convert the physical command-line address to a higher-half virtual address */
            parsed.cmdline = mod->cmdline ? (const char *)ADD_KERNEL_OFFSET((uintptr_t)mod->cmdline) : "";

            cb(&parsed, i, data);
        }
    }

    return mbi->mods_count;
}
