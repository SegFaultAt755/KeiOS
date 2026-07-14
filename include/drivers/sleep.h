#pragma once

/* SPDX-License-Identifier: GPLv3 */
/* Copyright (C) 2026 KeiOS Developers */

#include <stdint.h>

/*
    Note:
        Using sleep or sleep_ms takes 100% CPU usage, that means everything is stopped until they return
        Better to use PIT in most of situations.
        Functions sleep_initialize and sleep_deinitialize are optional, they enable and disable channel 2
        which is working for pc speaker
*/

void sleep_initialize(void);
void sleep(uint16_t ticks);
void sleep_ms(uint32_t ms);
void sleep_deinitialize(void);
