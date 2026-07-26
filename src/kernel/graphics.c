/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "kernel/graphics.h"

#include "drivers/display.h"
#include "drivers/terminal.h"

#if defined(__i386__) || defined(_M_IX86)
#include "arch/x86/mem.h"
#include "arch/x86/vmm.h"
#endif

#include "kernel/multiboot.h"
#include "kernel/qemu.h"

int get_graphics_type(struct multiboot_info *mbi) {
    if (mbi->flags & MULTIBOOT_INFO_FRAMEBUFFER_INFO) {
        if (mbi->framebuffer_type == GRAPHICS_TYPE_FRAMEBUFFER)
            return GRAPHICS_TYPE_FRAMEBUFFER;

        return GRAPHICS_TYPE_TEXT_MODE;
    }

    return GRAPHICS_TYPE_VGA_PALETTE;
}

int graphics_initialize(struct multiboot_info *mbi) {
    auto graphics = get_graphics_type(mbi);
    if (graphics == GRAPHICS_TYPE_TEXT_MODE) {
        /* Initialize VGA text mode */
        vga_init_text();
        terminal_initialize((uint16_t *)VGA_TEXT_MEMORY, VGA_TEXT_WIDTH, VGA_TEXT_HEIGHT);
    } else if (graphics == GRAPHICS_TYPE_FRAMEBUFFER) {
        struct display_info info;
        info.flags = mbi->flags;
        info.width = mbi->framebuffer_width;
        info.height = mbi->framebuffer_height;
        info.pitch = mbi->framebuffer_pitch;
        info.bpp = mbi->framebuffer_bpp;

        auto phys_addr = (uint32_t)mbi->framebuffer_addr;
        constexpr uint32_t virt_addr = 0xE0000000;
        auto fbo_size = info.pitch * info.height;

        qemu_printf(QEMU_DRV, QEMU_INFO, "Framebuffer address info: (physical: %p, virtual: %p, FBO size: %d)",
                    phys_addr, virt_addr, fbo_size);

        auto map_success = true;

        /* Loop through the entire size of the framebuffer and map it page by page */
        for (auto offset = 0u; offset < fbo_size; offset += PAGE_SIZE) {
            if (!vmm_map_page(virt_addr + offset, phys_addr + offset, PTE_PRESENT | PTE_RW | PTE_PWT)) {
                map_success = false;
                break;
            }
        }

        if (!map_success) {
            qemu_printf(QEMU_DRV, QEMU_ERROR, "Failed to map linear framebuffer to virtual memory");
            info.lfb_addr = nullptr;
        }

        info.lfb_addr = (uint32_t *)virt_addr;

        display_initialize(info);
        display_clear(0x00'14'14'14);
    }

    return graphics;
}
