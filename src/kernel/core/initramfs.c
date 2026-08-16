/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include "kernel/core/initramfs.h"
#include "kernel/qemu.h"
#include "libkern/string.h"

uint8_t *exec_init = nullptr;
uint32_t exec_init_size = 0;

void cpio_callback_function(const char *name, [[maybe_unused]] struct cpio_header header, const uint8_t *data,
                            [[maybe_unused]] size_t data_len, [[maybe_unused]] void *user_context) {
    if (data_len != 0) /* Skip directories */
        qemu_printf(QEMU_KERN, QEMU_INFO, "[CPIO] Found a file: (name: %s, size: %d)", name, data_len);

    const char *exec_name = "bin/init.bin";
    if (strcmp(name, exec_name) == 0) {
        exec_init = (uint8_t *)data;
        exec_init_size = data_len;
    }
}
