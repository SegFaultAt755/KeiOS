/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "kernel/core/cpio.h"
#include "kernel/panic.h"
#include "kernel/qemu.h"
#include "libkern/string.h"

static const uint8_t *init_binary;
static size_t init_binary_size;

void cpio_cb(const char *name, [[maybe_unused]] struct cpio_header hdr, const uint8_t *data, size_t data_len,
             [[maybe_unused]] void *ctx) {
    if (data_len != 0) /* Directories have no file data */
        qemu_printf(QEMU_KERN, QEMU_INFO, "[CPIO] Found file: name=%s size=%u", name, data_len);

    if (strcmp(name, "bin/init.bin") == 0) {
        KERNEL_ASSERT(init_binary == nullptr, "Duplicate init program",
                      "The initramfs contains more than one bin/init.bin entry");
        init_binary = data;
        init_binary_size = data_len;
    }
}

const uint8_t *cpio_get_init_binary(size_t *size) {
    if (!size) {
        return nullptr;
    }

    *size = init_binary_size;
    return init_binary;
}
