/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "drivers/graphics/framebuffer.h"

#include "arch/x86/mem.h"
#include "arch/x86/vmm.h"

static struct framebuffer_info active_framebuffer;
static bool framebuffer_ready;

int framebuffer_init(struct framebuffer_info info) {
    if (info.addr == nullptr || info.width == 0 || info.height == 0 || info.bpp != 32 || info.pitch == 0 ||
        (info.pitch & 3u) != 0 || info.width > UINT32_MAX / 4 || info.pitch < info.width * 4)
        return 1;

    const uint64_t framebuffer_size = (uint64_t)info.pitch * info.height;
    const uintptr_t physical_address = (uintptr_t)info.addr;
    const uintptr_t physical_page = physical_address & ~(uintptr_t)(PAGE_SIZE - 1);
    const uint32_t page_offset = physical_address - physical_page;
    const uint64_t map_size = framebuffer_size + page_offset;
    constexpr uintptr_t virtual_base = 0xE0000000;
    constexpr uint64_t virtual_size = 0x10000000;

    if (framebuffer_size == 0 || map_size > virtual_size ||
        (uint64_t)physical_address + framebuffer_size > (uint64_t)UINT32_MAX + 1)
        return 1;

    const uint32_t page_count = (uint32_t)((map_size + PAGE_SIZE - 1) / PAGE_SIZE);
    for (uint32_t page = 0; page < page_count; page++) {
        const uint32_t offset = page * PAGE_SIZE;
        if (!vmm_map_page((uint32_t)virtual_base + offset, (uint32_t)physical_page + offset,
                          PTE_PRESENT | PTE_RW | PTE_PWT))
            return 1;
    }

    info.addr = (uint32_t *)(virtual_base + page_offset);
    active_framebuffer = info;
    framebuffer_ready = true;
    return 0;
}

void framebuffer_clear(struct color32 c) {
    if (!framebuffer_ready)
        return;

    volatile uint32_t *pixels = (volatile uint32_t *)active_framebuffer.addr;
    const uint32_t color = color32_to_int(c);
    const uint32_t pixel_count = (active_framebuffer.pitch / sizeof(uint32_t)) * active_framebuffer.height;
    for (uint32_t index = 0; index < pixel_count; index++)
        pixels[index] = color;
}

void framebuffer_draw_pixel(struct color32 c, uint32_t x, uint32_t y) {
    if (!framebuffer_ready || x >= active_framebuffer.width || y >= active_framebuffer.height)
        return;

    const uint32_t stride = active_framebuffer.pitch / sizeof(uint32_t);
    volatile uint32_t *pixels = (volatile uint32_t *)active_framebuffer.addr;
    pixels[y * stride + x] = color32_to_int(c);
}

void framebuffer_fill_rect(struct color32 c, uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
    if (!framebuffer_ready || x >= active_framebuffer.width || y >= active_framebuffer.height || width == 0 ||
        height == 0)
        return;

    if (width > active_framebuffer.width - x)
        width = active_framebuffer.width - x;
    if (height > active_framebuffer.height - y)
        height = active_framebuffer.height - y;

    const uint32_t stride = active_framebuffer.pitch / sizeof(uint32_t);
    const uint32_t color = color32_to_int(c);
    volatile uint32_t *pixels = (volatile uint32_t *)active_framebuffer.addr;
    for (uint32_t row = 0; row < height; row++) {
        volatile uint32_t *destination = pixels + (y + row) * stride + x;
        for (uint32_t column = 0; column < width; column++)
            destination[column] = color;
    }
}
