/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "kernel/graphics.h"

#include "drivers/graphics/framebuffer.h"
#include "drivers/ipc.h"
#include "drivers/terminal.h"
#include "libkern/memory.h"

#include "kernel/multiboot.h"
#include "kernel/qemu.h"

enum gfx_operation {
    GFX_OP_CLEAR = 1,
    GFX_OP_PIXEL = 2,
    GFX_OP_FILL_RECT = 3,
};

struct gfx_request {
    uint32_t request_id;
    uint32_t operation;
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;
    uint32_t color;
};

static constexpr uint32_t GFX_IPC_SENDER = 1;
static uint32_t gfx_ipc_endpoint;
static uint32_t next_request_id = 1;
bool gfx_initialized = false;

void gfx_set_ipc_endpoint(uint32_t endpoint) {
    gfx_ipc_endpoint = endpoint;
}

static int gfx_process_request(const struct ipc_message *message, uint32_t *request_id) {
    if (message->sender != GFX_IPC_SENDER || message->length != sizeof(struct gfx_request))
        return IPC_INVALID;

    struct gfx_request request;
    memcpy(&request, message->data, sizeof(request));
    const struct color32 color = gen_color32(request.color);
    switch (request.operation) {
    case GFX_OP_CLEAR:
        framebuffer_clear(color);
        break;
    case GFX_OP_PIXEL:
        framebuffer_draw_pixel(color, request.x, request.y);
        break;
    case GFX_OP_FILL_RECT:
        framebuffer_fill_rect(color, request.x, request.y, request.width, request.height);
        break;
    default:
        return IPC_INVALID;
    }

    *request_id = request.request_id;
    return IPC_OK;
}

static int gfx_send_request(struct gfx_request request) {
    if (!gfx_initialized || gfx_ipc_endpoint == 0)
        return IPC_INVALID;

    request.request_id = next_request_id++;
    if (next_request_id == 0)
        next_request_id = 1;

    int result = ipc_send(gfx_ipc_endpoint, GFX_IPC_SENDER, (const uint8_t *)&request, sizeof(request));
    if (result != IPC_OK)
        return result;

    struct ipc_message message;
    do {
        result = ipc_receive(gfx_ipc_endpoint, &message);
        if (result != IPC_OK)
            return result;

        uint32_t processed_request_id;
        result = gfx_process_request(&message, &processed_request_id);
        if (result == IPC_OK && processed_request_id == request.request_id)
            return IPC_OK;
    } while (true);
}

int gfx_clear(struct color32 color) {
    return gfx_send_request((struct gfx_request){.operation = GFX_OP_CLEAR, .color = color32_to_int(color)});
}

int gfx_draw_pixel(struct color32 color, uint32_t x, uint32_t y) {
    return gfx_send_request(
        (struct gfx_request){.operation = GFX_OP_PIXEL, .x = x, .y = y, .color = color32_to_int(color)});
}

int gfx_fill_rect(struct color32 color, uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
    return gfx_send_request((struct gfx_request){.operation = GFX_OP_FILL_RECT,
                                                 .x = x,
                                                 .y = y,
                                                 .width = width,
                                                 .height = height,
                                                 .color = color32_to_int(color)});
}

int get_graphics_type(struct multiboot_info *mbi) {
    if (mbi == nullptr)
        return GRAPHICS_TYPE_TEXT_MODE;

    if (mbi->flags & MULTIBOOT_INFO_FRAMEBUFFER_INFO) {
        if (mbi->framebuffer_type == GRAPHICS_TYPE_FRAMEBUFFER)
            return GRAPHICS_TYPE_FRAMEBUFFER;

        return GRAPHICS_TYPE_TEXT_MODE;
    }

    return GRAPHICS_TYPE_VGA_PALETTE;
}

int gfx_init(struct multiboot_info *mbi) {
    auto graphics = get_graphics_type(mbi);
    gfx_initialized = false;
    if (graphics != GRAPHICS_TYPE_FRAMEBUFFER) {
        /* Initialize VGA text mode for text and palette boot modes. */
        vga_init_text();
        terminal_init((uint16_t *)VGA_TEXT_MEMORY, VGA_TEXT_WIDTH, VGA_TEXT_HEIGHT);
    } else if (graphics == GRAPHICS_TYPE_FRAMEBUFFER) {
        struct framebuffer_info info;
        info.addr = (uint32_t *)(uintptr_t)mbi->framebuffer_addr;
        info.flags = mbi->flags;
        info.width = mbi->framebuffer_width;
        info.height = mbi->framebuffer_height;
        info.pitch = mbi->framebuffer_pitch;
        info.bpp = mbi->framebuffer_bpp;

        if (mbi->framebuffer_addr <= UINT32_MAX)
            gfx_initialized = framebuffer_init(info) == 0;

        if (gfx_initialized && gfx_clear(gen_color32(0)) != IPC_OK)
            gfx_initialized = false;

        if (gfx_initialized)
            qemu_printf(QEMU_DRV, QEMU_OK, "Framebuffer initialized: (flags: %d, width: %d, height: %d)", info.flags,
                        info.width, info.height);
        else
            qemu_printf(QEMU_DRV, QEMU_ERROR, "Failed to initialize framebuffer or GFX IPC");
    }

    return graphics;
}
