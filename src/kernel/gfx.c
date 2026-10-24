/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "kernel/gfx.h"

#include "drivers/display.h"
#include "drivers/graphics/framebuffer.h"
#include "drivers/terminal.h"
#include "kernel/ipc.h"
#include "libkern/memory.h"

#include "kernel/multiboot.h"
#include "kernel/qemu.h"

enum gfx_operation {
    GFX_OP_CLEAR = 1,
    GFX_OP_PIXEL = 2,
    GFX_OP_FILL_RECT = 3,
    GFX_OP_TEXT = 4,
};

struct gfx_request {
    uint32_t request_id;
    uint32_t operation;
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;
    uint32_t color;
    char text[GFX_TEXT_MAX_BYTES + 1];
};

_Static_assert(sizeof(struct gfx_request) <= IPC_MAX_PAYLOAD, "GFX request exceeds IPC payload size");

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
    switch (request.operation) {
    case GFX_OP_CLEAR:
        display_clear(request.color);
        break;
    case GFX_OP_PIXEL:
        display_draw_pixel((int32_t)request.x, (int32_t)request.y, request.color);
        break;
    case GFX_OP_FILL_RECT:
        display_fill_rect((int32_t)request.x, (int32_t)request.y, request.width, request.height, request.color);
        break;
    case GFX_OP_TEXT: {
        size_t text_length = 0;
        while (text_length < sizeof(request.text) && request.text[text_length] != '\0')
            text_length++;
        if (text_length == sizeof(request.text) ||
            display_draw_text_static((int32_t)request.x, (int32_t)request.y, request.text, request.color) != 0)
            return IPC_INVALID;
        break;
    }
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

int gfx_draw_text(struct color32 color, uint32_t x, uint32_t y, const char *text) {
    if (text == nullptr)
        return IPC_INVALID;

    struct gfx_request request = {
        .operation = GFX_OP_TEXT,
        .x = x,
        .y = y,
        .color = color32_to_int(color),
    };
    size_t text_length = 0;
    while (text_length < sizeof(request.text) && text[text_length] != '\0')
        text_length++;
    if (text_length == sizeof(request.text))
        return IPC_INVALID;

    memcpy(request.text, text, text_length);
    return gfx_send_request(request);
}

int gfx_get_type(struct multiboot_info *mbi) {
    if (mbi == nullptr)
        return GFX_TYPE_TEXT_MODE;

    if (mbi->flags & MULTIBOOT_INFO_FRAMEBUFFER_INFO) {
        if (mbi->framebuffer_type == GFX_TYPE_FRAMEBUFFER)
            return GFX_TYPE_FRAMEBUFFER;

        return GFX_TYPE_TEXT_MODE;
    }

    return GFX_TYPE_VGA_PALETTE;
}

int gfx_init(struct multiboot_info *mbi) {
    auto gfx_type = gfx_get_type(mbi);
    gfx_initialized = false;
    if (gfx_type != GFX_TYPE_FRAMEBUFFER) {
        /* Initialize VGA text mode for text and palette boot modes. */
        vga_init_text();
        terminal_init((uint16_t *)VGA_TEXT_MEMORY, VGA_TEXT_WIDTH, VGA_TEXT_HEIGHT);
    } else if (gfx_type == GFX_TYPE_FRAMEBUFFER) {
        struct framebuffer_info framebuffer = {
            .addr = (uint32_t *)(uintptr_t)mbi->framebuffer_addr,
            .flags = mbi->flags,
            .width = mbi->framebuffer_width,
            .height = mbi->framebuffer_height,
            .pitch = mbi->framebuffer_pitch,
            .bpp = mbi->framebuffer_bpp,
        };

        if (mbi->framebuffer_addr <= UINT32_MAX && framebuffer_init(framebuffer) == 0) {
            struct display_info display = {
                .lfb_addr = framebuffer_get_addr(),
                .flags = framebuffer.flags,
                .width = framebuffer.width,
                .height = framebuffer.height,
                .pitch = framebuffer.pitch,
                .bpp = framebuffer.bpp,
            };
            gfx_initialized = display_init(display) == 0;
        }

        if (gfx_initialized && gfx_clear(gen_color32(0)) != IPC_OK)
            gfx_initialized = false;

        if (gfx_initialized)
            qemu_printf(QEMU_DRV, QEMU_OK, "Framebuffer initialized: (flags: %d, width: %d, height: %d)",
                        framebuffer.flags, framebuffer.width, framebuffer.height);
        else
            qemu_printf(QEMU_DRV, QEMU_ERROR, "Failed to initialize framebuffer or GFX IPC");
    }

    return gfx_type;
}
