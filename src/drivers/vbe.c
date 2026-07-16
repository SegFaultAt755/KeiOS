/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "drivers/vbe.h"
#if defined(__i386__) || defined(_M_IX86)
#include "arch/x86/mem.h"
#include "arch/x86/vmm.h"
#else
#error "Unsupported architecture! (i386 is available)"
#endif
#include "kernel/qemu.h"

static uint32_t *lfb = nullptr;
static uint32_t width = 0;
static uint32_t height = 0;
static uint32_t pitch = 0;
static uint8_t bpp = 0;

void vbe_initialize(struct multiboot_info *mbi) {
    if (!(mbi->flags & (1 << 12))) {
        qemu_printf(QEMU_DRV, QEMU_ERROR, "No framebuffer was provided by GRUB for VBE");
        return;
    }

    width = mbi->framebuffer_width;
    height = mbi->framebuffer_height;
    pitch = mbi->framebuffer_pitch;
    bpp = mbi->framebuffer_bpp;

    qemu_printf(QEMU_DRV, QEMU_INFO, "VBE info: (width: %d, height: %d, pitch: %d, bpp: %d)", width, height, pitch,
                bpp);

    uint32_t phys_addr = (uint32_t)mbi->framebuffer_addr;
    uint32_t virt_addr = VBE_VIRTUAL_LFB_START;
    uint32_t fbo_size = pitch * height;

    qemu_printf(QEMU_DRV, QEMU_INFO, "VBE address info: (physical: 0x%x, virtual: 0x%x, FBO size: %d)", phys_addr,
                virt_addr, fbo_size);

    bool map_success = true;

    /* Loop through the entire size of the framebuffer and map it page by page */
    for (uint32_t offset = 0; offset < fbo_size; offset += PAGE_SIZE) {
        if (!vmm_map_page(virt_addr + offset, phys_addr + offset, PTE_PRESENT | PTE_RW | PTE_PWT)) {
            map_success = false;
            break;
        }
    }

    if (!map_success) {
        qemu_printf(QEMU_DRV, QEMU_ERROR, "Failed to map VBE framebuffer to virtual memory");
        lfb = nullptr;
        return;
    }

    lfb = (uint32_t *)virt_addr;
}

void vbe_set_pixel(int x, int y, uint32_t color) {
    if (x < 0 || x >= (int)width || y < 0 || y >= (int)height) {
        qemu_printf(QEMU_DRV, QEMU_ERROR, "Pixel position is out of bounds {pos %d:%d, bounds %d:%d}", x, y, (int)width,
                    (int)height);
        return;
    }

    uint32_t index = (y * (pitch / 4)) + x;
    lfb[index] = color;
}
