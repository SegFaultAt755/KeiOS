#pragma once

/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include <stdint.h>

#define IPC_MAX_PAYLOAD 256

enum ipc_status {
    IPC_OK = 0,
    IPC_INVALID = 1,
    IPC_FULL = 2,
    IPC_EMPTY = 3,
    IPC_NOT_INITIALIZED = 4,
    IPC_INVALID_ENDPOINT = 5,
};

typedef enum ipc_status ipc_status_t;

struct ipc_message {
    uint32_t sender;
    uint32_t length;
    uint8_t  data[IPC_MAX_PAYLOAD];
};

ipc_status_t ipc_init(void);
uint32_t     ipc_endpoint_create(void);
ipc_status_t ipc_send(uint32_t endpoint, uint32_t sender, const uint8_t *data, uint32_t length);
ipc_status_t ipc_receive(uint32_t endpoint, struct ipc_message *message);
