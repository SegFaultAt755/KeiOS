#pragma once

/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include <stdint.h>

#define IPC_MAX_PAYLOAD 64

#define IPC_OK      0
#define IPC_INVALID 1
#define IPC_FULL    2
#define IPC_EMPTY   3

struct ipc_message {
    uint32_t sender;
    uint32_t length;
    uint8_t  data[IPC_MAX_PAYLOAD];
};

int32_t  ipc_init(void);
uint32_t ipc_endpoint_create(void);
int32_t  ipc_send(uint32_t endpoint, uint32_t sender, const uint8_t *data, uint32_t length);
int32_t  ipc_receive(uint32_t endpoint, struct ipc_message *message);