/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "kernel/core/cpio.h"
#include "kernel/qemu.h"

void cpio_cb(const char *name, [[maybe_unused]] struct cpio_header hdr, [[maybe_unused]] const uint8_t *data,
             [[maybe_unused]] size_t data_len, [[maybe_unused]] void *ctx) {
    if (data_len != 0) /* Directories have no file data */
        qemu_printf(QEMU_KERN, QEMU_INFO, "[CPIO] Found file: name=%s size=%u", name, data_len);
}
