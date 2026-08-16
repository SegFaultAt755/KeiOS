#pragma once

/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include <stdint.h>
#include <stddef.h>

struct cpio_info {
    const uint8_t *base_addr;
    size_t size;
};

struct cpio_header {
    uint32_t mode;
    uint32_t uid;
    uint32_t gid;
    uint32_t mtime;
    uint32_t filesize;
    uint32_t namesize;
};

typedef void(*cpio_callback)(
    const char *name, struct cpio_header header,
    const uint8_t *data, size_t data_len, void *user_context
);

int cpio_init(struct cpio_info info);
int cpio_parse(cpio_callback callback, void *user_context);

size_t cpio_get_offset(void);
size_t cpio_get_total_size(void);
const uint8_t *cpio_get_base_addr(void);

void cpio_set_offset(size_t new_offset);
void cpio_set_buffer(const uint8_t *new_addr, size_t size);
